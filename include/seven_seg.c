#include "seven_seg.h"
#include "shift_register.h"

/**
 * Takes a normal number from 0-9 and converts it to the correct combination of pins to light up each number.*/
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
 * Retrieves the correct pins to light up each number and write the values to the shift register.*/
void display_on_seven_seg(uint8_t num){
    uint8_t display_val = get_display_val_from_u8(num);
    for(uint8_t i = 0; i < 8; i++){
        uint8_t val = display_val & 1 << (7 - i);
        write_to_shift_register(val);
    }
}