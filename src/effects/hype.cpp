#include "hype.hpp"
#include "drawHelper.hpp"

#include <FastLED.h>

uint8_t Effects::Hype::addRing() {
    return addEffect({
        .drawRing = [](color* render_data, uint8_t pos, uint8_t lng, effect* effect) -> void {
            color frd[60];
            CHSV hsv;
            CRGB rgb;
            hsv.setHSV(0, 255, 255);
            constexpr int sol = 12;
            uint8_t a = ((millis() % (255 * sol)) / sol) % 255;
            for(uint8_t i = 0; i < 60; i++) {
                uint8_t e = i * 4.25 + a;
                hsv.hue = e;
                hsv2rgb_rainbow(hsv, rgb);
                frd[i] = {.r=rgb.r, .g=rgb.g, .b=rgb.b};
            }
            for(uint8_t i = 0, j = pos; i < lng; i++, j++) render_data[j] = frd[i];
        }
    });
}

uint8_t Effects::Hype::addColor() {
    return addFunction([](uint8_t pos) -> color {
        if(pos == 5) return {.r=0, .g=0, .b=0};
        constexpr int sol = 100;
        switch ((millis() % (6 * sol)) / sol) {
        case 0: return {.r=255, .g=0, .b=0};
        case 1: return {.r=255, .g=255, .b=0};
        case 2: return {.r=0, .g=255, .b=0};
        case 3: return {.r=0, .g=255, .b=255};
        case 4: return {.r=0, .g=0, .b=255};
        case 5: return {.r=255, .g=0, .b=255};
        }
        return {.r=0, .g=0, .b=0};
    });
}

uint8_t Effects::Hype::addTransition() {
    return addFunction([](color* render_data, color* effect_a, color* effect_b, long ms_since_start, uint8_t pos, uint8_t lng) -> bool {
        color frd[148];
        constexpr int duration = 2000;
        for(uint8_t i = 0; i < 148; i++) frd[i] = effect_a[i];
        for(uint8_t i = 0; i < min((60 * (int) ms_since_start) / duration, 60); i++) frd[i] = effect_b[i];
        for(uint8_t i = 60; i < min(((88 * (int) ms_since_start) / duration) + 60, 148); i++) frd[i] = effect_b[i];
        for(uint8_t i = 0, j = pos; i < lng; i++, j++) render_data[j] = frd[i];
        return ms_since_start > duration;
    });
}
