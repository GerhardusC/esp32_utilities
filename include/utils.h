#include "driver/gpio.h"
#include "esp_timer.h"

struct DisplayVal {
    uint8_t tens;
    uint8_t ones;
};

struct DisplayVal convert_number_to_display_val(uint8_t num);

void wait_us_blocking(uint32_t micros_to_wait);