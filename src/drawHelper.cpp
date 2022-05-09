#include "drawHelper.hpp"

void fill(color* render_data, uint8_t pos, uint8_t lng, color col) {
    for(uint8_t i = pos; i < pos + lng; i++) render_data[i] = col;
}

uint8_t printChar(color* render_data, const char var, uint8_t position, color on, color off) {
    uint8_t idx = 0;
    if(var == ' ') idx = 1;
    if(var >= 'a' && var <= 'z') idx = var - 'a' + 12;
    if(var >= 'A' && var <= 'Z') idx = var - 'A' + 12;
    if(var >= '0' && var <= '9') idx = var - '0' + 2;
    if(var == ':') idx = 38;
    if(var == '-') idx = 39;
    if(var == '_') idx = 40;
    if(var == '^') idx = 41;
    if(var == '!') idx = 42;
    if(var == '.') idx = 43;
    if(var == '?') idx = 44;
    if(var == '%') idx = 45;
    if(var == '"') idx = 46;
    if(var == '\'') idx = 47;
    if(var == ',') idx = 48;
    if(var == ';') idx = 49;
    if(var == '~') idx = 50;
    if(var == '=') idx = 51;
    const static uint16_t chars[] = {
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
        0b0000000011101110, // a
        0b0000000011111000, // b
        0b0000000001110010, // c
        0b0000000010111100, // d
        0b0000000011110010, // e
        0b0000000011100010, // f
        0b0000000001111010, // g
        0b0000000011101000, // h
        0b0000000001100000, // i
        0b0000000000011100, // j
        0b0111111001110001, // k
        0b0000000001110000, // l
        0b0000000000101010, // m
        0b0000000010101000, // n
        0b0000000010111000, // o
        0b0000000011100110, // p
        0b0000000011001110, // q
        0b0111111000111111, // r
        0b0111101111101111, // s
        0b0000000011110000, // t
        0b0000000001111100, // u
        0b0000000000111000, // v
        0b0000000001010100, // w
        0b0110110000110001, // x
        0b0000000011011100, // y
        0b0111011111011111, // z
        0b0000000000110001, // :
        0b0000000010000000, // -
        0b0000000000010000, // _
        0b0000000001000110, // ^
        0b0000000001011001, // !
        0b0000000001000001, // .
        0b0111011010011111, // ?
        0b0111011001011001, // %
        0b0000000001000100, // "
        0b0000000001000000, // '
        0b0000000000100000, // ,
        0b0001011000000001, // ;
        0b0110010000010001, // ~
        0b0000000010000010, // =
    };
    const uint16_t chr = chars[idx];
    if(chr & 1) { // 14 segment
        for(uint8_t i = 0; i < 14; i++) render_data[i + position] = ((chr >> (i + 1)) & 1) ? on : off;
        return 14;
    } else if ((chr >> 15) & 1) { // double 7 segment
        for(uint8_t i = 0; i < 28; i++) render_data[i + position] = ((chr >> ((i & 0xFFFE) / 2 + 1)) & 1) ? on : off;
        return 28;
    } else { // normal 7 segment
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

void doCharPrint(uint8_t &textI, const uint8_t textMaxLength, const char* text, color* rd, uint8_t &ci, const color on, const color off) {
    if(textI < textMaxLength && text[textI] != '\0') ci += printChar(rd, text[textI++], ci, on, off);
}

void checkColon(uint8_t &textI, const uint8_t textMaxLength, const char* text, color* rd, uint8_t &ci, const color on, const color off, bool &doColon) {
    if(textI < textMaxLength && text[textI] != '\0') {
        if(text[textI] == ':') {
            doColon = true;
            textI++;
        } else ci += printChar(rd, text[textI++], ci, on, off);
    }
}

uint8_t drawCoustomText(color* render_data, const char* text, uint8_t textMaxLength, uint8_t pos, uint8_t length, color on, color off) {
    color *rd = (color *)malloc((14*7)*sizeof(color));
    fill(rd, 0, 14*7, off);
    uint8_t textI = 0;
    uint8_t ci = 0;
    bool col0 = false;
    bool col1 = false;
    if(ci == 14 * 0) doCharPrint(textI, textMaxLength, text, rd, ci, on, off);
    if(ci == 14 * 1) doCharPrint(textI, textMaxLength, text, rd, ci, on, off);
    if(ci == 14 * 2) checkColon(textI, textMaxLength, text, rd, ci, on, off, col0);
    if(ci == 14 * 2) doCharPrint(textI, textMaxLength, text, rd, ci, on, off);
    if(ci == 14 * 3) doCharPrint(textI, textMaxLength, text, rd, ci, on, off);
    if(ci == 14 * 4) checkColon(textI, textMaxLength, text, rd, ci, on, off, col1);
    if(ci == 14 * 4) doCharPrint(textI, textMaxLength, text, rd, ci, on, off);
    if(ci == 14 * 5) doCharPrint(textI, textMaxLength, text, rd, ci, on, off);
    fill(rd, 14*6 + 0, 2, col0 ? on : off);
    fill(rd, 14*6 + 2, 2, col1 ? on : off);
    for(uint8_t i = 0, j = pos; i < min(length, (uint8_t) (14*7)); i++, j++) render_data[j] = rd[i];
    free(rd);
    return ci;
}
