#include "drawHelper.hpp"

void fill(color* render_data, uint8_t pos, uint8_t len, color col) {
    for(uint8_t i = pos; i < pos + len; i++) render_data[i] = col;
}

uint8_t printChar(color* render_data, const char var, uint8_t position, color on, color off) {
    uint8_t idx = 0;
    if(var == ' ') idx = 1;
    if(var >= 'a' && var <= 'z') idx = var - 'a' + 12;
    if(var >= 'A' && var <= 'Z') idx = var - 'A' + 12;
    if(var >= '0' && var <= '9') idx = var - '0' + 2;
    uint16_t chars[] = {
        /*
            0b0'xxxxxxx'gfedcba'0:
             aa 
            f  b
            f  b
             gg 
            e  c
            e  c
             dd 

            0b0'nmlkjih'gfedcba'1:
             ab 
            l  c
            k  d
             mn 
            j  e
            i  f
             hg 

            0b1'nmlkjih'gfedcba'0:
             aa    hh 
            f  b  m  i
            f  b  m  i
             gg    nn 
            e  c  l  j
            e  c  l  j
             dd    kk 
        */
        0b0100110101001011, // invalid char
        0b0000000000000000, // off
        0b0000000001111110, // 0
        0b0000000000001100, // 1
        0b0000000010110110, // 2
        0b0000000010011110, // 3
        0b0000000011001100, // 4
        0b0000000011011010, // 5
        0b0000000011111010, // 6
        0b0000000000001110, // 7
        0b0000000011111110, // 8
        0b0000000011011110, // 9
    };
    uint16_t chr = chars[idx];
    if(chr & 1) { // 14 seg
        for(uint8_t i = 0; i < 14; i++) render_data[i + position] = ((chr >> (i + 1)) & 1) ? on : off;
        return 14;
    } else if ((chr >> 15) & 1) { // double space 7 seg
        for(uint8_t i = 0; i < 28; i++) render_data[i + position] = ((chr >> ((i & 0xFFFE) / 2 + 1)) & 1) ? on : off;
        return 28;
    } else { // normal 7 seg
        for(uint8_t i = 0; i < 14; i++) render_data[i + position] = ((chr >> ((i & 0xFFFE) / 2 + 1)) & 1) ? on : off;
        return 14;
    }
}

color fadeToBlack(color c, uint16_t numerator, uint16_t denominator) {
    return {
        .r=(uint8_t) min(255, ((uint16_t) c.r * numerator) / denominator),
        .g=(uint8_t) min(255, ((uint16_t) c.g * numerator) / denominator),
        .b=(uint8_t) min(255, ((uint16_t) c.b * numerator) / denominator),
    };
}
