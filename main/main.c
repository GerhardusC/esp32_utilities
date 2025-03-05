#include <stdio.h>
//  ets_delay_us(int) comes from here...;
#include <rom/ets_sys.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
// Logging: ESP_LOGI("", "", "");
#include "esp_event.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_SSID CONFIG_WIFI_SSID;
#define WIFI_PWD CONFIG_WIFI_PASSWORD;
/* Wifi connect signal group*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define DATA_LINE 19
#define POW_LINE 21
#define SHIFT_REGISTER_VCC 22
#define SHIFT_REGISTER_A 23
#define SHIFT_REGISTER_CLOCK 4

#define DHT_METER_TIMER 2

// TODO!
void setup_wifi() {
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&config);
}

uint16_t wait_for_pin_state(gpio_num_t pin, uint32_t timeout, uint8_t expected_state){
    // Set as input pin to read from.
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    for(int i = 0; i < timeout; i += DHT_METER_TIMER){
        // Wait one cycle apparently for jitter.
        ets_delay_us(DHT_METER_TIMER);
        if(gpio_get_level(pin) == expected_state){
            return i;
        };
    }
    ESP_LOGE("Err", "Pin wait timeout");
    return 0;
}

void toggle_shift_register_clock() {
    gpio_set_level(SHIFT_REGISTER_CLOCK, 1);
    // ets_delay_us(5);
    gpio_set_level(SHIFT_REGISTER_CLOCK, 0);
    // ets_delay_us(5);
}

void write_to_shift_register(uint8_t val) {
    // Set data pin to val
    gpio_set_level(SHIFT_REGISTER_A, val);
    // ets_delay_us(5);

    // Tick clock
    toggle_shift_register_clock();

    // reset data pin
    gpio_set_level(SHIFT_REGISTER_A, 0);
    // ets_delay_us(5);
}

struct Temp_reading {
    uint16_t temp_sig;
    uint16_t temp_dec;
    uint16_t hum_sig;
    uint16_t hum_dec;
    uint8_t err;
};

void read_temp(gpio_port_t data_pin, struct Temp_reading *measurement) {
    // Reset error state.
    measurement->err = 0;
    // Low for 18 us on sig line.
    gpio_set_direction(data_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(data_pin, 0);
    ets_delay_us(19000);
    gpio_set_level(data_pin, 1);
    gpio_set_direction(data_pin, GPIO_MODE_INPUT);

    // Read response.

    // Phase 1: Wait 20-40 ms for downpull.
    if(wait_for_pin_state(data_pin, 40, 0) == 0){
        ESP_LOGI("Something went wrong at:", "Phase 1 wait for pull down.");
        measurement->err = 1;
        return;
    };

    // Phase 2: Wait for pull down by sensor
    if(wait_for_pin_state(data_pin, 88, 1) == 0){
        ESP_LOGI("Something went wrong at:", "Phase 2 wait for pull down by sensor.");
        measurement->err = 1;
        return;
    };

    // Phase 3: Wait for pull down by sensor
    if(wait_for_pin_state(data_pin, 88, 0) == 0){
        ESP_LOGI("Something went wrong at:", "Phase 2 wait for pull down by sensor.");
        measurement->err = 1;
        return;
    };

    uint8_t data[5];

    for(uint8_t i = 0; i < 40; i++){
        // measure low duration
        uint16_t base_dur = wait_for_pin_state(data_pin, 72, 1);
        // measure high duration
        uint16_t bit_dur = wait_for_pin_state(data_pin, 60, 0);

        uint8_t bit_index = i / 8;
        uint8_t num_byte = i % 8;
        if (num_byte == 0){
            data[bit_index] = 0;
        } 

        bool bit_value = bit_dur > base_dur;

        uint8_t current_byte = bit_value << (7 - num_byte);

        data[bit_index] = data[bit_index] | current_byte;
    };

    measurement->hum_sig = data[0];
    measurement->hum_dec = data[1];
    measurement->temp_sig = data[2];
    measurement->temp_dec = data[3];

    return;
}

void temp_task() {
    struct Temp_reading measurement = { 0, 0, 0, 0, 0 };
    while(1){
        read_temp(DATA_LINE, &measurement);
        if(!measurement.err){
            ESP_LOGI("Results:", "Humidity: %d, Temp: %d", measurement.hum_sig, measurement.temp_sig);
        } else {
            ESP_LOGI("ERROR:", "Thermometer error");
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void setup_thermometer() {
    gpio_reset_pin(POW_LINE);
    gpio_reset_pin(DATA_LINE);
    gpio_set_direction(DATA_LINE, GPIO_MODE_OUTPUT);
    gpio_set_direction(POW_LINE, GPIO_MODE_OUTPUT);

    gpio_set_level(POW_LINE, 1);
    gpio_set_level(DATA_LINE, 1);
}

void setup_shift_register() {
    gpio_reset_pin(SHIFT_REGISTER_VCC);
    gpio_set_direction(SHIFT_REGISTER_VCC, GPIO_MODE_OUTPUT);
    gpio_set_level(SHIFT_REGISTER_VCC, 1);

    gpio_reset_pin(SHIFT_REGISTER_A);
    gpio_set_direction(SHIFT_REGISTER_A, GPIO_MODE_OUTPUT);

    gpio_reset_pin(SHIFT_REGISTER_CLOCK);
    gpio_set_direction(SHIFT_REGISTER_CLOCK, GPIO_MODE_OUTPUT);
}

void clear_shift_register() {
    gpio_reset_pin(5);
    gpio_set_direction(5, GPIO_MODE_OUTPUT);
    gpio_set_level(5, 1);
    gpio_set_level(5, 0);
    gpio_set_level(5, 1);
}

void setup() {
    // gpio_reset_pin(2);
    // gpio_set_direction(2, GPIO_MODE_OUTPUT);

    setup_shift_register();
    setup_thermometer();

}

void display_on_seven_seg(uint8_t num){
    switch(num){
        case 0:
            // 00111111 = 0
            uint8_t zero = 0b00111111;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = zero & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
        case 1:
            // 00110000 = 1
            uint8_t one = 0b00110000;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = one & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
        case 2:
            // 01011011 = 2
            uint8_t two = 0b01011011;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = two & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
        case 3:
            // 01111001 = 3
            uint8_t three = 0b01111001;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = three & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
        case 4:
            // 01110100 = 4
            uint8_t four = 0b01110100;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = four & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
        case 5:
            // 01101101 = 5
            uint8_t five = 0b01101101;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = five & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
        case 6:
            // 01101111 = 6
            uint8_t six = 0b01101111;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = six & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
        case 7:
            // 00111000 = 7
            uint8_t seven = 0b00111000;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = seven & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
        case 8:
            // 01111111 = 8
            uint8_t eight = 0b01111111;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = eight & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
        case 9:
            // 01111101 = 9
            uint8_t nine = 0b01111101;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = nine & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
        default:
            uint8_t dot = 0b10000000;
            for(uint8_t i = 0; i < 8; i++){
                uint8_t val = dot & 1 << (7 - i);
                write_to_shift_register(val);
            }
            break;
    }
}

void app_main(void) {
    setup();
    xTaskCreate(temp_task, "Temp reading task", 5000, NULL, 1, NULL);


    clear_shift_register();

    int i = 0;

    while(1) {
        // Use to reset.
        // gpio_set_level(2, 1);
        
        vTaskDelay(100);
        display_on_seven_seg(i);

        i++;
        if(i > 10){
            i = 0;
        };
    }
}