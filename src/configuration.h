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

#define MQTT_HOST "cb.twitchbridge.de"
#define MQTT_USER "user"
#define MQTT_PASSWORD "password"



class Configuration
{
public:
    void setupWifiPortal(String hostName, bool configPortal);
    void connectionGuard();

    uint16_t getUniverse() {return universe.toInt();};
    uint16_t getMaxmilliamp() {return maxmilliamp.toInt();};
    const char *getMQTTHost() { return mqttHost.c_str(); };
    const char *getMQTTUser() { return mqttUser.c_str(); };
    const char *getMQTTPassword() { return mqttPassword.c_str(); };


private:
    String universe = String(UNIVERSE);
    String maxmilliamp = String(LED_MAX_MILLIAMP);
    String mqttHost = MQTT_HOST;
    String mqttUser = MQTT_USER;
    String mqttPassword = MQTT_PASSWORD;
    bool isMaster = false;

    void save();
    void setupSPIFF();
    static void enableSave();
    static void saveParamsCallback();
    static void saveConfigCallback();
    static bool shouldSave;
};


#endif // CONFIGURATION_H__
