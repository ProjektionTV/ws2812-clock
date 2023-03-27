#ifndef SETTINGS_H__
#define SETTINGS_H__

#define LED_PIN 5
#define NUM_LEDS 148
#define LED_MAX_MILLIAMP 500
#define COLOR_ORDER GRB
#define CHIPSET WS2812B

#define MAX_MQTT_CONNECT_TRYS 3

#define E131_ENABLED
#define UNIVERSE 1

#define CLOCK_BRIGHTNESS 42
#define E131_BRIGHTNESS 255

#define SEGMENTOFFSET 60
#define COLONOFFSET 144

#define UNIVERSE 1                      // First DMX Universe to listen for
#define UNIVERSE_COUNT 1                // Total number of Universes to listen for, starting at UNIVERSE
#define UNIVERSE_LENGTH 144

#define COLOR_DIGITS CRGB(0, 0, 255)
#define COLOR_COLON CRGB(255, 255, 0)
#define COLOR_SECONDS CRGB(0, 255, 0)
#define COLOR_SECONDS5 CRGB(255, 0, 0)

#define DEBUG_SERIAL true

#endif // SETTINGS_H__
