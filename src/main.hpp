#ifndef _MAIN_HPP_
#define _MAIN_HPP_

#include <Arduino.h>
#include <WiFiManager.h>

#include "types.hpp"
#include "settings.hpp"

extern effect currEffect;

extern bool isTransitiing;
extern transition currTransition;
extern effect currTransitionTarget;
extern long transitionStart;

extern ringEffect ringEffects[NUM_RING_EFFECTS];
extern middEffect middEffects[NUM_MIDD_EFFECTS];
typedef color color_function(uint8_t pos);
extern color_function *colorEffects[NUM_COLO_EFFECTS];
typedef bool transition_function(color* render_data, color* effect_a, color* effect_b, long ms_since_start, uint8_t pos, uint8_t lng);
extern transition_function *transitions[NUM_TRAN_EFFECTS];

extern WiFiManager wifiManager;


extern char customMessage[MAX_CUSTOM_MESSAGE_LENGHT];
extern unsigned long customMessageSet;
extern unsigned long customMessageDuration;

extern struct tm tm;
extern bool drawColon;

/*
    adds ringEffect `eff` to ring effect list
    return ringeffectid
*/
uint8_t addEffect(ringEffect eff);
/*
    adds midd Effect `eff` to midd effect list
    return middeffectid
*/
uint8_t addEffect(middEffect eff);
/*
    adds color function `fun` to color function list
    return color function id
*/
uint8_t addFunction(color_function* func);
/*
    adds transition `trans` to transition list
    return trasitionid
*/
uint8_t addFunction(transition_function* trans);

/*
    initiates Transition
    if a effect is 255 the current effect will be cept
*/
void initTransition(uint8_t transitionID, uint8_t ringEffectId = 255, uint8_t middEffectId = 255, uint8_t colorEffectId = 255);

void setLedBrightness(uint8_t brightness);

#endif /* _MAIN_HPP_ */
