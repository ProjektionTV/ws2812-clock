#ifndef _DRAWHELPER_HPP_
#define _DRAWHELPER_HPP_

#include <Arduino.h>

#include "types.hpp"

/*
    fills a part of render data
    WARNING: there is no check if the render data is big enough
*/
void fill(color* render_data, uint8_t pos, uint8_t lng, color col);

/*
    prints a char to render data to given position
    WARNING: there is no check if the render data is big enough
    required led space: 14/28
    
    returns: how many leds where drawn
*/
uint8_t printChar(color* render_data, const char var, uint8_t position, color on, color off);

/*
    returns: c * numerator / denominator
*/
color fadeToBlack(color c, uint16_t numerator, uint16_t denominator);

/*
    draws text to render data
    returns: amount of space used Internally
*/
uint8_t drawCoustomText(color* render_data, const char* text, uint8_t textMaxLength, uint8_t pos, uint8_t length, color on, color off);

#endif /* _DRAWHELPER_HPP_ */
