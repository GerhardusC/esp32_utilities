#include "shift_register.h"

#define SHIFT_REGISTER_VCC 22
#define SHIFT_REGISTER_A 23
#define SHIFT_REGISTER_CLOCK 4

void toggle_shift_register_clock() {
    gpio_set_level(SHIFT_REGISTER_CLOCK, 1);
    gpio_set_level(SHIFT_REGISTER_CLOCK, 0);
}

void write_to_shift_register(uint8_t val) {
    // Set data pin to val
    gpio_set_level(SHIFT_REGISTER_A, val);
    vTaskDelay(5);
    // Tick clock
    toggle_shift_register_clock();
    vTaskDelay(5);
    // reset data pin
    gpio_set_level(SHIFT_REGISTER_A, 0);
    vTaskDelay(5);
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