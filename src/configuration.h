#ifndef CONFIGURATION_H__
#define CONFIGURATION_H__

#ifndef CONFIG_ESP32_PHY_MAX_TX_POWER
    #define CONFIG_ESP32_PHY_MAX_TX_POWER CONFIG_ESP_PHY_MAX_WIFI_TX_POWER
#endif

#include <settings.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

#ifdef ESP32
#include <SPIFFS.h>
#endif


class Configuration
{
public:
    void setupWifiPortal(String hostName, bool configPortal);
    void connectionGuard();

    uint16_t getUniverse() {return universe.toInt();};
    uint16_t getMaxmilliamp() {return maxmilliamp.toInt();};

private:
    String universe = String(UNIVERSE);
    String maxmilliamp = String(LED_MAX_MILLIAMP);

    void save();
    void setupSPIFF();
    static void enableSave();
    static void saveParamsCallback();
    static void saveConfigCallback();
    static bool shouldSave;
};


#endif // CONFIGURATION_H__
