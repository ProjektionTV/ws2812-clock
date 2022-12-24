#include <Arduino.h>
#include <settings.h>
#include <configuration.h>
#include <ArduinoOTA.h>
#include <Time.h>
#include <FastLED.h>
#include <ESPAsyncE131.h>
#include <ESPNtpClient.h>
#include <version.h>

#define MY_NTP_SERVER "de.pool.ntp.org"
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"

extern Configuration config;
ESPAsyncE131 e131(UNIVERSE_COUNT);

CRGB leds[NUM_LEDS];

CRGB colorDigits = COLOR_DIGITS;
CRGB colorColon = COLOR_COLON;
CRGB colorSeconds = COLOR_SECONDS;
CRGB colorSeconds5 = COLOR_SECONDS5;

void draw();
bool oncePerHalfSecond(bool *lowerHalf);
bool e131RX();
void ledInit();
void printClock(bool showSecondsFlag, bool drawColonFlag);
void printDigit(uint8_t value, uint8_t position);
void showColon(int flag);
void showSeconds(uint8_t seconds);
void secondsRingInit();
void clearScreen();
void printVersionInfo();
void printConfigStatus();

void clearScreen()
{
  FastLED.clear();
  delay(1);
  FastLED.show();
}

void printConfigStatus()
{
  FastLED.clear();
  for(int i=0; i<NUM_LEDS; i++)
  {
    leds[i] = CRGB(255, 0, 0);
  }
  delay(1);
  FastLED.show();  
}

void printVersionInfo()
{
  FastLED.clear();

  printDigit(VERSION_PATCH % 10, 5);
  printDigit((VERSION_PATCH / 10) % 10, 4);
  
  printDigit(VERSION_MINOR % 10, 3);
  printDigit((VERSION_MINOR / 10) % 10, 2);
  
  printDigit(VERSION_MAJOR % 10, 1);
  printDigit((VERSION_MAJOR / 10) % 10, 0);
  delay(1);
  FastLED.show();
}

void secondsRingInit()
{
  time_t now;
  struct tm tm;
  
  time(&now);
  localtime_r(&now, &tm);

  uint16_t seconds = tm.tm_sec + 1;

  FastLED.clearData();
  for(int i=seconds; i<(seconds+60);i++)
  {
    showSeconds(i%60);
    showSeconds(i%60);
  }  
}

void ledInit()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_MAX_MILLIAMP);
  FastLED.setBrightness(CLOCK_BRIGHTNESS);
  FastLED.clear();

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(255, 255, 255);
    FastLED.show();
    delay(1);
  }
  delay(500);

  for (int i = 0; i < SEGMENTOFFSET; i++)
  {
    leds[i] = colorSeconds;
    FastLED.show();
    delay(1);
  }
  for (int i = SEGMENTOFFSET; i < COLONOFFSET; i++)
  {
    leds[i] = colorDigits;
    FastLED.show();
    delay(1);
  }
  for (int i = COLONOFFSET; i < NUM_LEDS; i++)
  {
    leds[i] = colorColon;
    FastLED.show();
    delay(1);
  }

  for (int i = 0; i < 255; i++)
  {
    fadeToBlackBy(leds, 60, 1);
    delay(1);
    FastLED.show();
  }
}

void showSeconds(uint8_t seconds)
{
  uint8_t secLed = (seconds + 59) % 60;

  if (seconds % 5)
    leds[secLed] = colorSeconds;
  else
    leds[secLed] = colorSeconds5;

  fadeToBlackBy(leds, 60, 3);
}

void showColon(int flag)
{
  if (flag)
  {
    leds[COLONOFFSET + 0] = colorColon;
    leds[COLONOFFSET + 1] = colorColon;
    leds[COLONOFFSET + 2] = colorColon;
    leds[COLONOFFSET + 3] = colorColon;
  }
  else
  {
    leds[COLONOFFSET + 0] = CRGB(0, 0, 0);
    leds[COLONOFFSET + 1] = CRGB(0, 0, 0);
    leds[COLONOFFSET + 2] = CRGB(0, 0, 0);
    leds[COLONOFFSET + 3] = CRGB(0, 0, 0);
  }
}

void printDigit(uint8_t value, uint8_t position)
{
  value = constrain(value, 0, 10);
  position = constrain(position, 0, 5);

  uint8_t segment[11][14] =
  {
    {// 0
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {// 1
      0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {// 2
      1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1},
    {// 3
      1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1},
    {// 4
      0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1},
    {// 5
      1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
    {// 6
      1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {// 7
      1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0},
    {// 8
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {// 9
      1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1},
    {// off
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
  };

  for (int i = 0; i < 14; i++)
  {
    if (segment[value][i])
    {
      leds[SEGMENTOFFSET + i + position * 14] = colorDigits;
    }
    else
    {
      leds[SEGMENTOFFSET + i + position * 14] = CRGB(0, 0, 0);
    }
  }
}

void printClock(bool showSecondsFlag, bool drawColonFlag)
{
  uint8_t seconds = 0;
  uint8_t minutes = 0;
  uint8_t hours = 0;
  time_t now;
  struct tm tm;
  
  time(&now);
  localtime_r(&now, &tm);

  seconds = tm.tm_sec;
  minutes = tm.tm_min;
  hours = tm.tm_hour;

  uint8_t second0 = seconds % 10;
  uint8_t second1 = seconds / 10;

  uint8_t minute0 = minutes % 10;
  uint8_t minute1 = minutes / 10;

  uint8_t hour0 = hours % 10;
  uint8_t hour1 = hours / 10;

  printDigit(second0, 5);
  printDigit(second1, 4);

  printDigit(minute0, 3);
  printDigit(minute1, 2);

  printDigit(hour0, 1);
  printDigit(hour1, 0);

  if(showSecondsFlag)
    showSeconds(seconds);
  showColon(drawColonFlag);
}

void setup()
{
  pinMode(0, INPUT_PULLUP); // low for config-mode; ESP32 = D0; ESP8266 = D3
  Serial.begin(115200);
  Serial.println();

  Serial.println("Press BootButton for ConfigPortal.");
  ledInit();
  clearScreen();
  printVersionInfo();
  delay(2000);

  bool config_flag = !digitalRead(0);
  
  if(config_flag)
    printConfigStatus();

  config.setupWifiPortal("ws2812-clock", config_flag);

  clearScreen();

  ArduinoOTA.begin();

  NTP.setTimeZone(MY_TZ);
  NTP.begin(MY_NTP_SERVER);

  if (e131.begin(E131_MULTICAST, config.getUniverse(), UNIVERSE_COUNT))   // Listen via Multicast
      Serial.println(F("Listening for data..."));
  else 
      Serial.println(F("*** e131.begin failed ***"));

}

bool e131RX()
{
  static uint32_t colorAverage = 0;

  if(!e131.isEmpty())
  {
    colorAverage = 0;

    e131_packet_t packet;
    e131.pull(&packet);

    uint8_t cnt = 0;

    for(uint8_t i=0; i<UNIVERSE_LENGTH;i++)
    {
      CRGB color = CRGB(packet.property_values[i*3+1], packet.property_values[i*3+2], packet.property_values[i*3+3]);
    
      CHSV hsvColor = rgb2hsv_approximate(color);
      if(hsvColor.v > 16)
      {
        colorAverage += hsvColor.hue;
        cnt++;
      }

      int o = map(i,0,143,0,59);
      leds[o] = color;
    }

    if(cnt)
      colorAverage /= cnt;

    colorDigits = CHSV(colorAverage, 255, 128);
    colorColon = CHSV(colorAverage+128, 255, 128);
    return true;
  }
  return false;
}

bool oncePerHalfSecond(bool *lowerHalf)
{
  static bool oldFlag = false;
  bool flag = (NTP.millis() % 1000) < 500;
  *lowerHalf = flag;

  if(flag != oldFlag)
  {
    oldFlag = flag;
    return true;
  }
  return false;
}

void draw()
{
  uint32_t ms = millis();
  static uint32_t lastMulticastRxMs = ms;
  static uint32_t lastDrawMs = ms;
  static bool showSecondsFlag = false;
  static bool firstTimeout = false;

  if(ms > (lastMulticastRxMs + 2000))
  {
    if(firstTimeout)
    {
      FastLED.setBrightness(CLOCK_BRIGHTNESS);
      secondsRingInit();
    }
    colorDigits = COLOR_DIGITS;
    colorColon = COLOR_COLON;
    showSecondsFlag = true;
    firstTimeout=false;
  }

  if(e131RX())
  {
    lastMulticastRxMs = ms;
    showSecondsFlag = false;
    firstTimeout=true;
    FastLED.setBrightness(E131_BRIGHTNESS);
    printClock(showSecondsFlag, (NTP.millis() % 1000) < 500);
    lastDrawMs = ms;
    FastLED.show();
  }
  else
  {
    bool drawColon = false;
    if((ms > lastDrawMs+10) && oncePerHalfSecond(&drawColon))
    {
      printClock(showSecondsFlag, drawColon);
      lastDrawMs = ms;
      FastLED.show();
    }
  }
}


void loop()
{
  config.connectionGuard();
  ArduinoOTA.handle();
  draw();
}
