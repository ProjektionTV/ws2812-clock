#ifndef _DRAWHELPER_HPP_
#define _DRAWHELPER_HPP_

#include <Arduino.h>

#include "types.hpp"

/*
    fills a part of renderdata
    WARNING: there is no check if the render data is big enought
*/
void fill(color* render_data, uint8_t pos, uint8_t len, color col);

/*
    prints a char to renderdata to given position
    WARNING: there is no check if the render data is big enought
    required led space: 14/28
    
    returns: how many leds where drawn
*/
uint8_t printChar(color* render_data, const char var, uint8_t position, color on, color off);

/*
    returns c * numerator / denominator
*/
color fadeToBlack(color c, uint16_t numerator, uint16_t denominator);

#endif /* _DRAWHELPER_HPP_ */
