#include <stdio.h>
#include <rom/ets_sys.h>
//  ets_delay_us(int) comes from here...;
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
// #include "esp_sleep.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
// Logging: ESP_LOGI("", "", "");
#define DATA_LINE 19
#define SIGNAL_LINE 21

#define METER_TIMER 2


uint16_t wait_for_pin_state(gpio_num_t pin, uint32_t timeout, uint8_t expected_state){
    // Set as input pin to read from.
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    for(int i = 0; i < timeout; i += METER_TIMER){
        // Wait one cycle apparently for jitter.
        ets_delay_us(METER_TIMER);
        if(gpio_get_level(pin) == expected_state){
            return i;
        };
    }
    return 0;
}

struct Temp_reading {
    uint16_t temp_sig;
    uint16_t temp_dec;
    uint16_t hum_sig;
    uint16_t hum_dec;
    uint8_t err;
};

struct Temp_reading read_temp() {
    struct Temp_reading res = {
        0,
        0,
        0,
        0,
        0,
    };
    // Low for 18 us on sig line.
    gpio_set_direction(DATA_LINE, GPIO_MODE_OUTPUT);
    gpio_set_level(DATA_LINE, 0);
    ets_delay_us(19000);
    gpio_set_level(DATA_LINE, 1);
    gpio_set_direction(DATA_LINE, GPIO_MODE_INPUT);

    // Read response.

    // Phase 1: Wait 20-40 ms for downpull.
    if(wait_for_pin_state(DATA_LINE, 40, 0) == 0){
        ESP_LOGI("Something went wrong at:", "Phase 1 wait for pull down.");
        res.err = 1;
        return res;
    };

    // Phase 2: Wait for pull down by sensor
    if(wait_for_pin_state(DATA_LINE, 88, 1) == 0){
        ESP_LOGI("Something went wrong at:", "Phase 2 wait for pull down by sensor.");
        res.err = 1;
        return res;
    };

    // Phase 3: Wait for pull down by sensor
    if(wait_for_pin_state(DATA_LINE, 88, 0) == 0){
        ESP_LOGI("Something went wrong at:", "Phase 2 wait for pull down by sensor.");
        res.err = 1;
        return res;
    };

    uint8_t data[5];

    for(uint8_t i = 0; i < 40; i++){
        // measure low duration
        uint16_t base_dur = wait_for_pin_state(DATA_LINE, 72, 1);
        // measure high duration
        uint16_t bit_dur = wait_for_pin_state(DATA_LINE, 60, 0);

        uint8_t bit_index = i / 8;
        uint8_t num_byte = i % 8;
        if (num_byte == 0){
            data[bit_index] = 0;
        } 

        bool bit_value = bit_dur > base_dur;

        uint8_t current_byte = bit_value << (7 - num_byte);

        data[bit_index] = data[bit_index] | current_byte;
    };

    res.hum_sig = data[0];
    res.hum_dec = data[1];
    res.temp_sig = data[2];
    res.temp_dec = data[3];

    return res;
}

void app_main(void)
{
    gpio_reset_pin(2);
    gpio_set_direction(2, GPIO_MODE_OUTPUT);

    gpio_reset_pin(SIGNAL_LINE);
    gpio_reset_pin(DATA_LINE);
    gpio_set_direction(SIGNAL_LINE, GPIO_MODE_OUTPUT);

    gpio_set_level(SIGNAL_LINE, 1);
    gpio_set_level(DATA_LINE, 1);

    while(1) {
        // Use to reset.
        struct Temp_reading reading = read_temp();
        ESP_LOGI("Results:", "Humidity: %d, Temp: %d", reading.hum_sig, reading.temp_sig);
        gpio_set_level(2, 1);
        vTaskDelay(100);
        gpio_set_level(2, 0);
        vTaskDelay(100);
    }
}