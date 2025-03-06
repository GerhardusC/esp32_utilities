#include "dht.h"
#define DHT_METER_TIMER 2
#define DATA_LINE 19
#define POW_LINE 21

void setup_thermometer() {
    gpio_reset_pin(POW_LINE);
    gpio_reset_pin(DATA_LINE);
    gpio_set_direction(DATA_LINE, GPIO_MODE_OUTPUT);
    gpio_set_direction(POW_LINE, GPIO_MODE_OUTPUT);

    gpio_set_level(POW_LINE, 1);
    gpio_set_level(DATA_LINE, 1);
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

void read_temp(struct Temp_reading *measurement) {
    // Reset error state.
    measurement->err = 0;
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
        measurement->err = 1;
        return;
    };

    // Phase 2: Wait for pull down by sensor
    if(wait_for_pin_state(DATA_LINE, 88, 1) == 0){
        ESP_LOGI("Something went wrong at:", "Phase 2 wait for pull down by sensor.");
        measurement->err = 1;
        return;
    };

    // Phase 3: Wait for pull down by sensor
    if(wait_for_pin_state(DATA_LINE, 88, 0) == 0){
        ESP_LOGI("Something went wrong at:", "Phase 2 wait for pull down by sensor.");
        measurement->err = 1;
        return;
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

    measurement->hum_sig = data[0];
    measurement->hum_dec = data[1];
    measurement->temp_sig = data[2];
    measurement->temp_dec = data[3];

    return;
}