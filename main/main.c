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

#include "dht.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#define WIFI_SSID CONFIG_WIFI_SSID;
#define WIFI_PWD CONFIG_WIFI_PASSWORD;
/* Wifi connect signal group*/
static EventGroupHandle_t s_wifi_event_group;

/* From the ESP docs re. WiFi: The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define DATA_LINE 19
#define POW_LINE 21
#define SHIFT_REGISTER_VCC 22
#define SHIFT_REGISTER_A 23
#define SHIFT_REGISTER_CLOCK 4


// TODO!
void setup_wifi() {
    wifi_init_config_t config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&config);
}


void toggle_shift_register_clock() {
    gpio_set_level(SHIFT_REGISTER_CLOCK, 1);
    gpio_set_level(SHIFT_REGISTER_CLOCK, 0);
}

void write_to_shift_register(uint8_t val) {
    // Set data pin to val
    gpio_set_level(SHIFT_REGISTER_A, val);
    // Tick clock
    toggle_shift_register_clock();
    // reset data pin
    gpio_set_level(SHIFT_REGISTER_A, 0);
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
        vTaskDelay(5000 / portTICK_PERIOD_MS);
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
    setup_shift_register();
    setup_thermometer();
}

/**
 * Takes a normal number from 0-9 and converts it to the correct combination of pins to light up each number.
 */
uint8_t get_display_val_from_u8(uint8_t num) {
    switch(num){
        case 0: return 0b00111111;
        case 1: return 0b00110000;
        case 2: return 0b01011011;
        case 3: return 0b01111001;
        case 4: return 0b01110100;
        case 5: return 0b01101101;
        case 6: return 0b01101111;
        case 7: return 0b00111000;
        case 8: return 0b01111111;
        case 9: return 0b01111101;
        default: return 0b10000000;
    }
}

/**
 * Retrieves the correct pins to light up each number and write the values to the shift register.
 */
void display_on_seven_seg(uint8_t num){
    uint8_t display_val = get_display_val_from_u8(num);
    for(uint8_t i = 0; i < 8; i++){
        uint8_t val = display_val & 1 << (7 - i);
        write_to_shift_register(val);
    }
}

void app_main(void) {
    setup();
    xTaskCreate(temp_task, "Temp reading task", 5000, NULL, 1, NULL);
    clear_shift_register();

    int i = 0;
    while(1) {
        vTaskDelay(100);
        display_on_seven_seg(i);
        i++;
        if(i > 10){
            i = 0;
        };
    }
}