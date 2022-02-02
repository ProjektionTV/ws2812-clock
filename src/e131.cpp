#include "e131.hpp"

#include <Arduino.h>
#include <ESPAsyncE131.h>
#include <Preferences.h>

#include "main.hpp"

#define UNIVERSE_COUNT 1
#define E131_DISPLAYTIME 3000

Preferences e131Preferences;
ESPAsyncE131 e131(UNIVERSE_COUNT);

bool useE131;
uint16_t e131Universe;

unsigned long lastDataReceived;

WiFiManagerParameter e131ParamOn("e131on", "E131 ON", "", 20, " type=\"checkbox\"");
WiFiManagerParameter e131ParamUniverse("e131universe", "E131 Universe", "", 5);

void initE131() {
    e131Preferences.begin("e131");
    e131Universe = e131Preferences.getShort("universe");
    useE131 = e131Preferences.getBool("use");
    e131Preferences.end();

    e131ParamOn.setValue((useE131 ? "' checked '" : ""), 20);
    e131ParamUniverse.setValue(String(e131Universe).c_str(), 5);

    wifiManager.addParameter(&e131ParamOn);
    wifiManager.addParameter(&e131ParamUniverse);

    lastDataReceived = millis() - E131_DISPLAYTIME;
}

void startE131() {
    if(useE131) {
        Serial.print("Starting E1.31 ...");
        if(e131.begin(E131_MULTICAST, e131Universe, UNIVERSE_COUNT)) {
            Serial.println("started");
        } else {
            Serial.println("couldn't start");
        }
    }
}

bool loopE131() {
    if(useE131) {
        if(!e131.isEmpty()) {
            lastDataReceived = millis();
            e131_packet_t packet;
            while(!e131.isEmpty()) {
                e131.pull(&packet);
                for(uint8_t i = 0; i < min(NUM_LEDS, NUM_LEDS); i++)
                    rd_c[i] = {.r=packet.property_values[i*3+1],.g=packet.property_values[i*3+2],.b=packet.property_values[i*3+3]};
            }
            drawClockFlag = true;
            return true;
        }
        return lastDataReceived + E131_DISPLAYTIME > millis();
    } else {
        return false;
    }
}

void saveE131() {
    useE131 = wifiManager.server->hasArg("e131on");
    e131Universe = wifiManager.server->hasArg("e131universe") ? wifiManager.server->arg("e131universe").toInt() : e131Universe;

    Serial.print("E131/on "); Serial.println(useE131 ? "true" : "false");
    Serial.print("E131/universe "); Serial.println(e131Universe);
    
    e131ParamOn.setValue((useE131 ? "' checked '" : ""), 20);

    e131Preferences.begin("e131");
    e131Preferences.putShort("universe", e131Universe);
    e131Preferences.putBool("use", useE131);
    e131Preferences.end();
}
