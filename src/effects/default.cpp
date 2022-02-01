#include "default.hpp"
#include "drawHelper.hpp"

namespace Effects {
    namespace Default {
        uint8_t digDrawn = 0;
        uint8_t digStart = 0;
    } // namespace Default
} // namespace Effects

uint8_t Effects::Default::addMidd() {
    return addEffect({
        .drawMidd = [](color* render_data, uint8_t pos, uint8_t lng, effect* effect) -> void {
            uint8_t seconds = 0;
            uint8_t minutes = 0;
            uint8_t hours = 0;

            seconds = tm.tm_sec;
            minutes = tm.tm_min;
            hours = tm.tm_hour;

            uint8_t second0 = seconds % 10 + '0';
            uint8_t second1 = seconds / 10 + '0';

            uint8_t minute0 = minutes % 10 + '0';
            uint8_t minute1 = minutes / 10 + '0';

            uint8_t hour0 = hours % 10 + '0';
            uint8_t hour1 = hours / 10 + '0';

            uint8_t index = 0;
            color frd[14*6+4];
            if(customMessageSet + customMessageDuration > millis()){
                drawCoustomText(frd, customMessage, 8, 0, 14*6+4, effect->getColor(7), effect->getColor(5));
            } else {
                color colA = effect->getColor(4);
                color colB = effect->getColor(5);
                index += printChar(frd, hour1, index, colA, colB);
                index += printChar(frd, hour0, index, colA, colB);
                index += printChar(frd, minute1, index, colA, colB);
                index += printChar(frd, minute0, index, colA, colB);
                index += printChar(frd, second1, index, colA, colB);
                index += printChar(frd, second0, index, colA, colB);
                fill(frd, index, 4, effect->getColor(5 + (drawColon ? 1 : 0)));
            }
            for(uint8_t i = 0, j = pos; i < lng; i++, j++) render_data[j] = frd[i];
        }
    });
}

uint8_t Effects::Default::addRing() {
    return addEffect({
        .drawRing = [](color* render_data, uint8_t pos, uint8_t lng, effect* effect) -> void {
            color frd[60];
            color colA = effect->getColor(0);
            color colB = effect->getColor(1);
            uint8_t sec = tm.tm_sec;
            uint8_t _60dSec = 60 - sec;
            uint8_t expos = digDrawn + digStart;
            if(digDrawn < 60) {
                uint8_t ndraw = sec < digStart ? sec - digStart + 60 : sec - digStart;
                digDrawn = ndraw < digDrawn ? 60 : ndraw;
            }
            if(digDrawn >= 60)
                for(uint8_t i = 0; i < 60; i++)
                    frd[i] = fadeToBlack(((i + 1) % 5) ? colA : colB, (i < sec ? _60dSec + i : i - sec) + 20, 80);
            else
                for(uint8_t i = 0; i < 60; i++)
                    frd[i] = fadeToBlack(((i + 1) % 5) ? colA : colB, ((i < digStart ? i + 60 : i) >= expos) ? 0 : ((i < sec ? _60dSec + i : i - sec) + 20), 80);
            for(uint8_t i = 0, j = pos; i < lng; i++, j++) render_data[j] = frd[i];
        }
    });
}

uint8_t Effects::Default::addColor() {
    return addFunction([](uint8_t pos) -> color {
        switch (pos) {
        case 0:
            return {.r=0, .g=255, .b=0};
        case 1:
            return {.r=255, .g=0, .b=0};
        case 2:
            return {.r=0, .g=0, .b=0};
        case 3:
            return {.r=0, .g=0, .b=0};

        case 4:
            return {.r=0, .g=0, .b=255};
        case 5:
            return {.r=0, .g=0, .b=0};
        case 6:
            return {.r=255, .g=255, .b=0};
        case 7:
            return {.r=0, .g=0, .b=255};
        
        default:
            return {.r=0, .g=0, .b=0};
        }
    });
}

uint8_t Effects::Default::addTransition() {
    return addFunction([](color* render_data, color* effect_a, color* effect_b, long ms_since_start, uint8_t pos, uint8_t lng) -> bool {
        for(uint8_t i = 0, j = pos; i < lng; i++, j++) render_data[j] = effect_b[j];
        return true;
    });
}

void Effects::Default::init() {
    digStart = tm.tm_sec;
}
