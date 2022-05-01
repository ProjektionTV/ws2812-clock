#include <Arduino.h>
#include <wifictrl.h>
#include <ArduinoOTA.h>
#include <Time.h>
#include <FastLED.h>
#include <ESPAsyncE131.h>

#define MY_NTP_SERVER "de.pool.ntp.org"
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"

#define LED_PIN 5
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define NUM_LEDS 148

#define SEGMENTOFFSET 60
#define COLONOFFSET 144

#define UNIVERSE 1                      // First DMX Universe to listen for
#define UNIVERSE_COUNT 1                // Total number of Universes to listen for, starting at UNIVERSE

ESPAsyncE131 e131(UNIVERSE_COUNT);

CRGB leds[NUM_LEDS];

CRGB colorDigits = CRGB(0, 0, 255);
CRGB colorColon = CRGB(255, 255, 0);
CRGB colorSeconds = CRGB(0, 255, 0);
CRGB colorSeconds5 = CRGB(255, 0, 0);

hw_timer_t *timer = NULL;
time_t now;
struct tm tm;

volatile bool halfSecondFlag = false;
volatile bool newClockDrawFlag = false;

void ledInit();
void printClock();
void printDigit(uint8_t value, uint8_t position);
void showColon(int flag);
void showSeconds(uint8_t seconds);
void waitingForNtpSync();

void IRAM_ATTR onTimer()
{
  halfSecondFlag = !halfSecondFlag;
  newClockDrawFlag = true;
}

void ledInit()
{
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(42);
  FastLED.clear();

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CRGB(255, 255, 255);
    FastLED.show();
    delay(10);
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

void printClock()
{
  uint8_t seconds = 0;
  uint8_t minutes = 0;
  uint8_t hours = 0;

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

  showSeconds(seconds);
  showColon(halfSecondFlag);

  FastLED.show();
}

void waitingForNtpSync()
{
  time(&now);
  localtime_r(&now, &tm);

  while (tm.tm_year <= 70)
  {
    time(&now);
    localtime_r(&now, &tm);
  }

  int sec = tm.tm_sec;
  while (sec == tm.tm_sec)
  {
    time(&now);
    localtime_r(&now, &tm);
  }
  delay(250);
}

void setup()
{
  Serial.begin(115200);
  Serial.println();

  ledInit();

  wifictrl.setupWifiPortal("ws2812-clock");

  ArduinoOTA.begin();

  configTime(0, 0, MY_NTP_SERVER); // 0, 0 because we will use TZ in the next line
  setenv("TZ", MY_TZ, 1);          // Set environment variable with your time zone
  tzset();

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 500000, true);

  waitingForNtpSync();

  timerAlarmEnable(timer);

  if (e131.begin(E131_MULTICAST, UNIVERSE, UNIVERSE_COUNT))   // Listen via Multicast
      Serial.println(F("Listening for data..."));
  else 
      Serial.println(F("*** e131.begin failed ***"));

}



void loop()
{
  wifictrl.check();
  ArduinoOTA.handle();
  static uint32_t lastMulticastRxMs = 0;

  if(!e131.isEmpty())
  {
    lastMulticastRxMs = millis();
    e131_packet_t packet;
    e131.pull(&packet);     // Pull packet from ring buffer
    
    // Serial.printf("Universe %u / %u Channels | Packet#: %u / Errors: %u / CH1: %u\n",
    //   htons(packet.universe),                 // The Universe for this packet
    //   htons(packet.property_value_count) - 1, // Start code is ignored, we're interested in dimmer data
    //   e131.stats.num_packets,                 // Packet counter
    //   e131.stats.packet_errors,               // Packet error counter
    //   packet.property_values[1]);             // Dimmer data for Channel 1

    for(uint8_t i=0; i<NUM_LEDS;i++)
    {
      leds[i] = CRGB(packet.property_values[i*3+1], packet.property_values[i*3+2], packet.property_values[i*3+3]);
    }
    FastLED.show();
  }


  if(millis() > (lastMulticastRxMs + 3000) && newClockDrawFlag)
  {
    newClockDrawFlag = false;
    time(&now);
    localtime_r(&now, &tm);
    printClock();
  }
}
