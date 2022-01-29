#include "main.hpp"

#include <Arduino.h>
#include <WiFi.h>
#include <FastLED.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>

#include "types.hpp"
#include "settings.hpp"
#include "drawHelper.hpp"
#include "mqtt.hpp"

#include "effects/default.hpp"

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

CRGB leds[NUM_LEDS];

ringEffect ringEffects[NUM_RING_EFFECTS];
middEffect middEffects[NUM_MIDD_EFFECTS];
color_function *colorEffects[NUM_COLO_EFFECTS];
transition_function *transitions[NUM_TRAN_EFFECTS];

char customMessage[MAX_CUSTOM_MESSAGE_LENGHT];
unsigned long customMessageSet = 0;

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
    currEffect._ringEffect = ringEffects + Effects::Default::addRing();
    currEffect._middEffect = middEffects + Effects::Default::addMidd();
    currEffect.getColor = colorEffects[Effects::Default::addColor()];
    currTransition.transition = transitions[Effects::Default::addTransition()];
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
    drawCoustomText(rd_c, "config", 6, o, 14*6 +4, on, off);
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
    initMqtt();
    wifiManager.setSaveParamsCallback([]() -> void {
        saveMqtt();
    });
    std::vector<const char *> menu = {"wifi","info","param","sep","restart","exit"};
    wifiManager.setMenu(menu);

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
    Serial.print("Current IP: ");
    Serial.println(WiFi.localIP().toString());
    // wifi connected

    addDefaultEffects();

    startMqtt();

    // OTA
#if ENABLE_OTA
    ArduinoOTA.setHostname(OTA_HOST);
    if (OTA_PORT != 0)
        ArduinoOTA.setPort(OTA_PORT);
    ArduinoOTA.begin();
#endif

    // ntp
    configTzTime(MY_TZ, MY_NTP_SERVER);
    getNtpSync();
    Effects::Default::init();
}

void loop() {
    drawColon = ((millis() - msdiffsec) % 1000) < 500;
    drawClockFlag = true;
    getLocalTime(&tm, 100);

    loopMqtt();

#if ENABLE_OTA
    ArduinoOTA.handle();
#endif

    if(drawClockFlag){
        drawClockFlag = false;
        drawClock();
    }
}
