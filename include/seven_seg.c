#include "seven_seg.h"
#include "shift_register.h"

/**
 * Takes a normal number from 0-9 and converts it to the correct combination of pins to light up each number.*/
uint8_t get_display_val_from_u8(uint8_t num) {
    switch(num){
        case 0: return 0b11101110;
        case 1: return 0b10000010;
        case 2: return 0b11001101;
        case 3: return 0b01101101;
        case 4: return 0b00101011;
        case 5: return 0b01100111;
        case 6: return 0b11100111;
        case 7: return 0b00101100;
        case 8: return 0b11101111;
        case 9: return 0b01101111;
        default: return 0b00010000;
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