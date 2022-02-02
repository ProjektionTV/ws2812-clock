#ifndef _STR_UTIL_HPP_
#define _STR_UTIL_HPP_

#include <Arduino.h>

unsigned int readNum(unsigned int &i, uint8_t *data, unsigned int length);
void skipToNum(String &str, uint8_t &i);
uint8_t readNum(String &str, uint8_t &i);

#endif /* _STR_UTIL_HPP_ */
