#include "driver/gpio.h"

struct DisplayVal {
    uint8_t tens;
    uint8_t ones;
};

struct DisplayVal convert_number_to_display_val(uint8_t num);