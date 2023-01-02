#include <configuration.h>

WiFiManager wifiManager;

Configuration config;

bool Configuration::shouldSave = false;

void Configuration::enableSave()
{
    shouldSave = true;
}

void Configuration::setupWifiPortal(String hostName, bool configPortal)
{

    setupSPIFF();

    WiFi.setHostname(hostName.c_str());
    wifiManager.setHostname(hostName.c_str());
    // wifiManager.resetSettings();

    wifiManager.setDarkMode(true);


    wifiManager.setDebugOutput(false);
    wifiManager.setConfigPortalTimeout(300);

    // WiFiManagerParameter custom_html("<p>This Is Custom HTML</p>"); // only custom html
    // wifiManager.addParameter(&custom_html);

    WiFiManagerParameter custom_universe("universe", "Universe", universe.c_str(), 5);
    wifiManager.addParameter(&custom_universe);

    WiFiManagerParameter custom_maxmilliamp("maxmilliamp", "max. mA", maxmilliamp.c_str(), 5);
    wifiManager.addParameter(&custom_maxmilliamp);

    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setSaveParamsCallback(saveParamsCallback);

    if (configPortal)
    {
        wifiManager.setConfigPortalBlocking(true);
        Serial.print("Starting ConfigPortal...");
        wifiManager.startConfigPortal(hostName.c_str());
    }
    else
    {
        Serial.print("Attempting WiFi connection... ");
        bool res = wifiManager.autoConnect(hostName.c_str(), NULL);
        if (!res)
        {
            Serial.println("failed! -> Reset");
            delay(2500);
            ESP.restart();
        }
        else
        {
            Serial.printf("connected, IP: %s\n", WiFi.localIP().toString().c_str());
        }             
    }
    universe = custom_universe.getValue();
    maxmilliamp = custom_maxmilliamp.getValue();

    if((universe.toInt() < 1) || (universe.toInt() > 255))
    {
        universe = String(UNIVERSE);
        shouldSave = true;
    }

    if(maxmilliamp.toInt() < 500)
    {
        maxmilliamp = String(LED_MAX_MILLIAMP);
        shouldSave = true;
    }

    if(shouldSave)
    {
        save();
    }

    Serial.println("Configuration completed!");
}


void Configuration::connectionGuard()
{
    if(!WiFi.isConnected())
    {
        Serial.print("\nAttempting WiFi reconnect");
        WiFi.begin();
        static uint8_t tries = 120;

        while(!WiFi.isConnected())
        {
            if(!tries--)
            {
                Serial.println("\nfailed! -> Reset");
                delay(2500);
                ESP.restart();
            }
            Serial.print('.');
            delay(1000);
        }
        //server.begin();
        Serial.println(" connected.");
        Serial.println(WiFi.localIP());
    }       
}

void Configuration::save()
{
    Serial.println("saving config");
#if ARDUINOJSON_VERSION_MAJOR >= 6
    DynamicJsonDocument json(1024);
#else
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.createObject();
#endif
    json["universe"] = universe;
    json["maxmilliamp"] = maxmilliamp;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("failed to open config file for writing");
    }

#if ARDUINOJSON_VERSION_MAJOR >= 6
    serializeJson(json, Serial);
    serializeJson(json, configFile);
#else
    json.printTo(Serial);
    json.printTo(configFile);
#endif
    configFile.close();
    // end save
}


void Configuration::setupSPIFF()
{
    Serial.println("mounting FS...");

#ifdef ESP32
    if (SPIFFS.begin(true))
#else
    if (SPIFFS.begin())
#endif
    {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            // file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile)
            {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);

#if ARDUINOJSON_VERSION_MAJOR >= 6
                DynamicJsonDocument json(1024);
                auto deserializeError = deserializeJson(json, buf.get());
                // serializeJson(json, Serial);
                if (!deserializeError)
                {
#else
                DynamicJsonBuffer jsonBuffer;
                JsonObject &json = jsonBuffer.parseObject(buf.get());
                json.printTo(Serial);
                if (json.success())
                {
#endif
                    // Serial.println("\nparsed json");
                    String strUniverse = json["universe"];
                    String strMaxmilliamp = json["maxmilliamp"];

                    universe = strUniverse;
                    maxmilliamp = strMaxmilliamp;

                    Serial.printf("Config Restored\n");
                    Serial.printf(" Universe:\t %s\n", universe.c_str());
                    Serial.printf(" max mA:\t %s\n", maxmilliamp.c_str());
                }
                else
                {
                    Serial.println("failed to load json config");
                }
                configFile.close();
            }
        }
    }
    else
    {
        Serial.println("failed to mount FS");
    }
}


void Configuration::saveParamsCallback()
{
    Serial.println("saveParamsCallback");
    enableSave();
    wifiManager.stopConfigPortal();
};

void Configuration::saveConfigCallback()
{
    Serial.println("saveConfigCallback");
    enableSave();
};