#include "strUtil.hpp"

unsigned int readNum(unsigned int &i, uint8_t *data, unsigned int length) {
    unsigned int t = 0;
    while(length > i && data[i] >= '0' && data[i] <= '9') {
        t *= 10;
        t += data[i] - '0';
        i++;
    }
    return t;
}

void skipToNum(String &str, uint8_t &i) {
    while(str.length() > i && (!(str.charAt(i) >= '0' && str.charAt(i) <= '9'))) i++;
}

uint8_t readNum(String &str, uint8_t &i) {
    skipToNum(str, i);
    uint8_t t = 0;
    while(str.length() > i && str.charAt(i) >= '0' && str.charAt(i) <= '9') {
        t *= 10;
        t += str.charAt(i) - '0';
        i++;
    }
    return t;
}
