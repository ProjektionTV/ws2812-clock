#include "main.hpp"

#include <Arduino.h>
#include <WiFi.h>
#include <FastLED.h>
#include <WiFiManager.h>

#include "types.hpp"
#include "settings.hpp"
#include "drawHelper.hpp"

#define LED_PIN 5
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define NUM_LEDS 148

#define SEGMENTOFFSET 60
#define COLONOFFSET 144

WiFiManager wifiManager;

struct tm tm;
uint16_t msdiffsec = 0;
long lastMSNtpSync = 0;

bool drawClockFlag = true;
bool drawColon = false;
uint8_t digDrawn = 0;
uint8_t digS = 0;

CRGB leds[NUM_LEDS];

ringEffect ringEffects[NUM_RING_EFFECTS];
middEffect middEffects[NUM_MIDD_EFFECTS];
color_function *colorEffects[NUM_COLO_EFFECTS];
transition_function *transitions[NUM_TRAN_EFFECTS];
char customMessage[MAX_CUSTOM_MESSAGE_LENGHT];

uint8_t ringEffectsAMT = 0;
uint8_t middEffectsAMT = 0;
uint8_t colorEffectsAMT = 0;
uint8_t transitionsAMT = 0;

effect currEffect = {
    ._ringEffect = ringEffects + 0,
    ._middEffect = middEffects + 0,
    .getColor = colorEffects[0],
};

bool isTransitiing = false;
transition currTransition = {
    .transition = transitions[0],
};
effect currTransitionTarget = {
    ._ringEffect = ringEffects + 0,
    ._middEffect = middEffects + 0,
    .getColor = colorEffects[0],
};
long transitionStart = 0;

color rd_c[NUM_LEDS];
color rd_t0[NUM_LEDS];
color rd_t1[NUM_LEDS];

void ledInit() {
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.clear();
    CRGB colorDigits = CRGB(0, 0, 255);
    CRGB colorColon = CRGB(255, 255, 0);
    CRGB colorSeconds = CRGB(0, 255, 0);
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB(255, 255, 255);
        FastLED.show();
        delay(10);
    }
    delay(500);
    for (int i = 0; i < SEGMENTOFFSET; i++) {
        leds[i] = colorSeconds;
        FastLED.show();
        delay(1);
    }
    for (int i = SEGMENTOFFSET; i < COLONOFFSET; i++) {
        leds[i] = colorDigits;
        FastLED.show();
        delay(1);
    }
    for (int i = COLONOFFSET; i < NUM_LEDS; i++) {
        leds[i] = colorColon;
        FastLED.show();
        delay(1);
    }
    for (int i = 0; i < 255; i++) {
        fadeToBlackBy(leds, 60, 1);
        delay(1);
        FastLED.show();
    }
}

uint8_t addEffect(ringEffect eff) {
    ringEffects[ringEffectsAMT++] = eff;
    return ringEffectsAMT - 1;
}
uint8_t addEffect(middEffect eff) {
    middEffects[middEffectsAMT++] = eff;
    return middEffectsAMT - 1;
}
uint8_t addFunction(color_function* func) {
    colorEffects[colorEffectsAMT++] = func;
    return colorEffectsAMT - 1;
}
uint8_t addFunction(transition_function* trans) {
    transitions[transitionsAMT++] = trans;
    return transitionsAMT - 1;
}

void initTransition(uint8_t transitionID, uint8_t ringEffectId, uint8_t middEffectId, uint8_t colorEffectId) {
    if(!~ringEffectId) currTransitionTarget._ringEffect = currEffect._ringEffect;
        else currTransitionTarget._ringEffect = ringEffects + ringEffectId;
    if(!~middEffectId) currTransitionTarget._middEffect = currEffect._middEffect;
        else currTransitionTarget._middEffect = middEffects + middEffectId;
    if(!~colorEffectId) currTransitionTarget.getColor = currEffect.getColor;
        else currTransitionTarget.getColor = colorEffects[colorEffectId];
    currTransition.transition = transitions[transitionID];
    transitionStart = millis();
    isTransitiing = true;
}

void addDefaultEffects() {
    currEffect._ringEffect = ringEffects + addEffect({
        .drawRing = [](color* render_data, uint8_t pos, uint8_t len, effect* effect) -> void {
            color frd[60];
            color colA = effect->getColor(0);
            color colB = effect->getColor(1);
            uint8_t sec = tm.tm_sec;
            uint8_t _60dSec = 60 - sec;
            uint8_t expos = digDrawn + digS;
            if(digDrawn < 60) {
                uint8_t ndraw = sec < digS ? sec - digS + 60 : sec - digS;
                digDrawn = ndraw < digDrawn ? 60 : ndraw;
            }
            if(digDrawn >= 60)
                for(uint8_t i = 0; i < 60; i++)
                    frd[i] = fadeToBlack(((i + 1) % 5) ? colA : colB, (i < sec ? _60dSec + i : i - sec) + 20, 80);
            else
                for(uint8_t i = 0; i < 60; i++)
                    frd[i] = fadeToBlack(((i + 1) % 5) ? colA : colB, ((i < digS ? i + 60 : i) >= expos) ? 0 : ((i < sec ? _60dSec + i : i - sec) + 20), 80);
            for(uint8_t i = 0, j = pos; i < len; i++, j++) render_data[j] = frd[i];
        }
    });
    currEffect._middEffect = middEffects + addEffect({
        .drawMidd = [](color* render_data, uint8_t pos, uint8_t len, effect* effect) -> void {
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
            index += printChar(frd, hour1, index, effect->getColor(4), effect->getColor(5));
            index += printChar(frd, hour0, index, effect->getColor(4), effect->getColor(5));
            index += printChar(frd, minute1, index, effect->getColor(4), effect->getColor(5));
            index += printChar(frd, minute0, index, effect->getColor(4), effect->getColor(5));
            index += printChar(frd, second1, index, effect->getColor(4), effect->getColor(5));
            index += printChar(frd, second0, index, effect->getColor(4), effect->getColor(5));
            fill(frd, index, 4, effect->getColor(6 + (drawColon ? 0 : 1)));
            for(uint8_t i = 0, j = pos; i < len; i++, j++) render_data[j] = frd[i];
        }
    });
    currEffect.getColor = colorEffects[addFunction([](uint8_t pos) -> color {
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
            return {.r=0, .g=0, .b=0};
        
        default:
            return {.r=0, .g=0, .b=0};
        }
    })];
    currTransition.transition = transitions[addFunction([](color* render_data, color* effect_a, color* effect_b, long ms_since_start, uint8_t pos, uint8_t lng) -> bool {
        for(uint8_t i = 0, j = pos; i < lng; i++, j++) render_data[j] = effect_b[j];
        return true;
    })];
}

void fastLEDdraw() {
    for(uint8_t i = 0; i < NUM_LEDS; i++) {
        color c = rd_c[i];
        leds[i] = CRGB(c.r, c.g, c.b);
    }
    FastLED.show();
}

void drawClock() {
    if(isTransitiing) {
        currEffect._ringEffect->drawRing(rd_t0, 0, SEGMENTOFFSET, &currEffect);
        currEffect._middEffect->drawMidd(rd_t0, SEGMENTOFFSET, NUM_LEDS - SEGMENTOFFSET, &currEffect);
        currTransitionTarget._ringEffect->drawRing(rd_t1, 0, SEGMENTOFFSET, &currEffect);
        currTransitionTarget._middEffect->drawMidd(rd_t1, SEGMENTOFFSET, NUM_LEDS - SEGMENTOFFSET, &currEffect);
        if(currTransition.transition(rd_c, rd_t0, rd_t1, millis() - transitionStart, 0, NUM_LEDS)) {
            isTransitiing = false;
            currEffect.getColor = currTransitionTarget.getColor;
            currEffect._middEffect = currTransitionTarget._middEffect;
            currEffect._ringEffect = currTransitionTarget._ringEffect;
        }
    } else {
        currEffect._ringEffect->drawRing(rd_c, 0, SEGMENTOFFSET, &currEffect);
        currEffect._middEffect->drawMidd(rd_c, SEGMENTOFFSET, NUM_LEDS - SEGMENTOFFSET, &currEffect);
    }
    fastLEDdraw();
}

void getNtpSync() {
    while (!getLocalTime(&tm));
    uint8_t sec = tm.tm_sec;
    while (sec == tm.tm_sec) getLocalTime(&tm, 100);
    msdiffsec = millis() % 1000;
    lastMSNtpSync = millis();
    Serial.print("msdiffsec: ");
    Serial.println(msdiffsec);
}

void handleBootButton() {
    bool a = false;
    if(!digitalRead(0)) a = true;
    delay(50);
    if(!digitalRead(0)) a = true;
    if(!a) return;
    int o = 60;
    color on = {.r=255,.g=255,.b=255,};
    color off = {.r=0,.g=0,.b=0,};
    o += printChar(rd_c, 'c', o, on, off);
    o += printChar(rd_c, 'o', o, on, off);
    o += printChar(rd_c, 'n', o, on, off);
    o += printChar(rd_c, 'f', o, on, off);
    o += printChar(rd_c, 'i', o, on, off);
    o += printChar(rd_c, 'g', o, on, off);
    fastLEDdraw();
    while(digitalRead(0));
    wifiManager.startConfigPortal(WIFI_AP_NAME, WIFI_AP_PASSWORD);
    ESP.restart();
}

void setup() {
    // setup serial
    Serial.begin(115200);
    Serial.println();

    ledInit();

    // wifi
    WiFi.mode(WIFI_STA);
    wifiManager.setDebugOutput(false);
    wifiManager.setShowStaticFields(true);

    pinMode(0, INPUT);
    handleBootButton();
    // auto connect
    bool res;
    if(!(res = wifiManager.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD))) {
        Serial.println("Faild to connect to Wifi!");
        delay(2500);
        ESP.restart();
    }
    Serial.println("WiFi connected...");
    // wifi connected

    addDefaultEffects();

    // ntp
    configTzTime(MY_TZ, MY_NTP_SERVER);
    getNtpSync();
    digS = tm.tm_sec;
}

void loop() {
    drawColon = ((millis() - msdiffsec) % 1000) < 500;
    drawClockFlag = true;
    getLocalTime(&tm, 100);

    if(drawClockFlag){
        drawClockFlag = false;
        drawClock();
    }
}
