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

    WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqttHost.c_str(), 40);
    wifiManager.addParameter(&custom_mqtt_server);

    WiFiManagerParameter custom_mqtt_user("user", "mqtt user", mqttUser.c_str(), 40);
    wifiManager.addParameter(&custom_mqtt_user);

    WiFiManagerParameter custom_mqtt_password("password", "mqtt password", mqttPassword.c_str(), 40);
    wifiManager.addParameter(&custom_mqtt_password);

    WiFiManagerParameter custom_universe("universe", "Universe", universe.c_str(), 5);
    wifiManager.addParameter(&custom_universe);

    WiFiManagerParameter custom_maxmilliamp("maxmilliamp", "max. mA", maxmilliamp.c_str(), 5);
    wifiManager.addParameter(&custom_maxmilliamp);

    char _custom_checkbox_master[32] = "type=\"checkbox\"";

    if(isMaster)
    {
        strcat(_custom_checkbox_master, " checked");
    }

    WiFiManagerParameter custom_checkbox_is_master("ismaster", "Master", "T", 2, _custom_checkbox_master, WFM_LABEL_AFTER);
    wifiManager.addParameter(&custom_checkbox_is_master);


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

    mqttHost = custom_mqtt_server.getValue();
    mqttUser = custom_mqtt_user.getValue();
    mqttPassword = custom_mqtt_password.getValue();    
    universe = custom_universe.getValue();
    maxmilliamp = custom_maxmilliamp.getValue();
    isMaster = (strncmp(custom_checkbox_is_master.getValue(), "T", 1) == 0);

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
    DynamicJsonDocument json(1024);
    json["mqtt_host"] = mqttHost;
    json["mqtt_user"] = mqttUser;
    json["mqtt_password"] = mqttPassword;    
    json["universe"] = universe;
    json["maxmilliamp"] = maxmilliamp;
    json["is_master"] = isMaster;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile)
    {
        Serial.println("failed to open config file for writing");
    }

    //serializeJson(json, Serial);
    serializeJson(json, configFile);
    configFile.close();
    // end save
}


void Configuration::setupSPIFF()
{
    Serial.println("mounting FS...");

    if (SPIFFS.begin(true))
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

                DynamicJsonDocument json(1024);
                auto deserializeError = deserializeJson(json, buf.get());
                // serializeJson(json, Serial);
                if (!deserializeError)
                {
                    // Serial.println("\nparsed json");
                    String host = json["mqtt_host"];
                    String user = json["mqtt_user"];
                    String password = json["mqtt_password"];                    
                    String strUniverse = json["universe"];
                    String strMaxmilliamp = json["maxmilliamp"];
                    bool ismaster = json["is_master"];

                    mqttHost = host;
                    mqttUser = user;
                    mqttPassword = password;
                    universe = strUniverse;
                    maxmilliamp = strMaxmilliamp;
                    isMaster = ismaster;

                    Serial.printf("Config Restored\n");
                    Serial.printf(" mqtt_host:\t %s\n", mqttHost.c_str());
                    Serial.printf(" mqtt_user:\t %s\n", mqttUser.c_str());
                    Serial.printf(" mqtt_password:\t <HIDDEN>\n");                    
                    Serial.printf(" Universe:\t %s\n", universe.c_str());
                    Serial.printf(" max mA:\t %s\n", maxmilliamp.c_str());
                    Serial.printf(" is_master:\t %d\n", isMaster);
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