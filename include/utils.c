#include "utils.h"

struct DisplayVal convert_number_to_display_val(uint8_t num){
    struct DisplayVal displayval = { 0,0 };

    if(num > 99) {
        displayval.tens = 9;
        displayval.ones = 9;
        return displayval;
    }

    displayval.tens = num / 10;
    displayval.ones = num % 10;
    return displayval;
}