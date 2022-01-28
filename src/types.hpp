#ifndef _TYPES_HPP_
#define _TYPES_HPP_

#include <Arduino.h>

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct effect;

struct ringEffect {
    void (*drawRing)(color* render_data, uint8_t pos, uint8_t len, effect* effect);
};
struct middEffect {
    void (*drawMidd)(color* render_data, uint8_t pos, uint8_t len, effect* effect);
};

struct effect {
    ringEffect* _ringEffect;
    middEffect* _middEffect;
    color (*getColor)(uint8_t pos); // pos: (0-3: ring, 4-7: midd)
};

struct transition {
    bool (*transition)(color* render_data, color* effect_a, color* effect_b, long ms_since_start); // rtn: is done
};

#endif /* _TYPES_HPP_ */
