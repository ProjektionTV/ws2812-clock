#include "mqtt.hpp"

#include <Arduino.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include <WiFiClientSecure.h>

#include "main.hpp"
#include "strUtil.hpp"
#include "e131.hpp"

PubSubClient psClient;
Preferences psPreferences;

uint8_t psCConf;
uint16_t psCPort;
String psCDomain;
String psCClientID;
String psCUser;
String psCPass;
String psCTopic;
bool useMqtt;

WiFiManagerParameter psParamDomain("psdomain", "Mqtt Host", "", 20);
WiFiManagerParameter psParamPort("psport", "Mqtt Port", "", 5);
WiFiManagerParameter psParamUser("psuser", "Mqtt User", "", 20);
WiFiManagerParameter psParamPass("pspass", "Mqtt Password", "", 20, " type=\"password\"");
WiFiManagerParameter psParamClId("psclid", "Mqtt Client Id", "", 20);
WiFiManagerParameter psParamTopic("pstopic", "Mqtt Topic", "", 20);

WiFiManagerParameter psParamOn("pson", "MQTT ON", "", 20, " type=\"checkbox\"");
WiFiManagerParameter psParamIsDomain("psisdomain", "MQTT IS DOMAIN", "", 20, " type=\"checkbox\"");
WiFiManagerParameter psParamSSL("psssl", "MQTT SSL", "", 20, " type=\"checkbox\"");
WiFiManagerParameter psParamAuth("psaouth", "MQTT AUTH", "", 20, " type=\"checkbox\"");


WiFiClientSecure wcs;
WiFiClient wc;

void connectMqtt() {
    uint8_t tryC = 0;
    while(!psClient.connected()) {
        Serial.print("attempting to connect to mqtt! ... ");
        if(((psCConf >> 1) & 1) ? psClient.connect(psCClientID.c_str(), psCUser.c_str(), psCPass.c_str()) : psClient.connect(psCClientID.c_str())) {
            Serial.println("connected to mqtt!");
            Serial.print("subscribing to: ");
            Serial.println(psCTopic);
            psClient.subscribe(psCTopic.c_str());
        } else {
            tryC++;
            if(tryC == 6) {
                useMqtt = false;
                Serial.println("Couldn't connect to Mqtt! disabling Mqtt!");
                break;
            }
        }
    }
}

void mqttCallback(char * topic, uint8_t *payload, unsigned int length) {
    Serial.print("[MQTT] [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.write(payload, length);
    Serial.println();
    unsigned int i = 0;
    uint8_t t_t = 0, t_r = 255, t_m = 255, t_c = 255;
    unsigned int textl = 0;
    while(i < length) {
        switch (payload[i++]) {
            case 'm': // custom message
                textl = readNum(i, payload, length);
                if(i < length && payload[i] == ' ') i++;
                for(uint8_t j = 0; j < min(textl, 8U); j++) customMessage[j] = (i < length) ? payload[i++] : '\0';
                for(uint8_t j = min(textl, 8U); j < 8; j++) customMessage[j] = '\0';
                if(textl > 8) i += textl - 8;
                customMessageSet = millis();
                break;
            case 'd': // custom message duration
                customMessageDuration = readNum(i, payload, length);
                break;
            case 'b': // led brightness
                setLedBrightness(readNum(i, payload, length));
                break;
            case 't': // transition transition id
                t_t = readNum(i, payload, length);
                break;
            case 'r': // transition ring id
                t_r = readNum(i, payload, length);
                break;
            case 'c': // transition color id
                t_c = readNum(i, payload, length);
                break;
            case 'i': // transition midd id
                t_m = readNum(i, payload, length);
                break;
            case 'e': // execute transition
                initTransition(t_t, t_r, t_m, t_c);
                break;
            case 's': // restart e1.31
                startE131();
                break;
        }
    }
}

void initMqtt() {
    psPreferences.begin("mqtt");
    psCDomain = psPreferences.getString("domain");
    psCClientID = psPreferences.getString("client");
    psCUser = psPreferences.getString("user");
    psCPass = psPreferences.getString("password");
    psCConf = psPreferences.getUChar("config");
    psCPort = psPreferences.getUShort("port");
    psCTopic = psPreferences.getString("topic");
    psPreferences.end();

    useMqtt = (psCConf >> 3) & 1;

    psParamOn.setValue((((psCConf >> 3) & 1) ? "' checked '" : ""), 20);
    psParamDomain.setValue(psCDomain.c_str(), 20);
    psParamIsDomain.setValue((((psCConf >> 2) & 1) ? "' checked '" : ""), 20);
    psParamPort.setValue(String(psCPort).c_str(), 5);
    psParamSSL.setValue((((psCConf >> 0) & 1) ? "' checked '" : ""), 20);
    psParamAuth.setValue((((psCConf >> 1) & 1) ? "' checked '" : ""), 20);
    psParamUser.setValue(psCUser.c_str(), 20);
    psParamPass.setValue(psCPass.c_str(), 20);
    psParamClId.setValue(psCClientID.c_str(), 20);
    psParamTopic.setValue(psCTopic.c_str(), 20);
    wifiManager.addParameter(&psParamOn);
    wifiManager.addParameter(&psParamDomain);
    wifiManager.addParameter(&psParamIsDomain);
    wifiManager.addParameter(&psParamPort);
    wifiManager.addParameter(&psParamSSL);
    wifiManager.addParameter(&psParamUser);
    wifiManager.addParameter(&psParamPass);
    wifiManager.addParameter(&psParamClId);
    wifiManager.addParameter(&psParamTopic);
}

void startMqtt() {
    if(useMqtt) {
        Serial.println("setting up Mqtt Client");
        if(psCConf & 1) {
            psClient.setClient(wcs);
        } else {
            psClient.setClient(wc);
        }
        if((psCConf >> 2) & 1) {
            psClient.setServer(psCDomain.c_str(), psCPort);
        } else {
            uint8_t i = 0;
            uint8_t a = readNum(psCDomain, i);
            uint8_t b = readNum(psCDomain, i);
            uint8_t c = readNum(psCDomain, i);
            uint8_t d = readNum(psCDomain, i);
            IPAddress ser(a, b, c, d);
            psClient.setServer(ser, psCPort);
        }
        psClient.setCallback(mqttCallback);
        connectMqtt();
    }
}

void saveMqtt() {
    psCClientID = wifiManager.server->hasArg("psclid") ? wifiManager.server->arg("psclid") : psCClientID;
    psCTopic = wifiManager.server->hasArg("pstopic") ? wifiManager.server->arg("pstopic") : psCTopic;
    psCUser = wifiManager.server->hasArg("psuser") ? wifiManager.server->arg("psuser") : psCUser;
    psCPass = wifiManager.server->hasArg("pspass") ? wifiManager.server->arg("pspass") : psCPass;
    psCDomain = wifiManager.server->hasArg("psdomain") ? wifiManager.server->arg("psdomain") : psCDomain;
    psCPort = wifiManager.server->hasArg("psport") ? wifiManager.server->arg("psport").toInt() : psCPort;
    psCConf = (psCConf & (~(1 << 3))) | (wifiManager.server->hasArg("pson") ? 1 << 3 : 0);
    psCConf = (psCConf & (~(1 << 2))) | (wifiManager.server->hasArg("psisdomain") ? 1 << 2 : 0);
    psCConf = (psCConf & (~(1 << 0))) | (wifiManager.server->hasArg("psssl") ? 1 << 0 : 0);
    psCConf = (psCConf & (~(1 << 1))) | (wifiManager.server->hasArg("psaouth") ? 1 << 1 : 0);

    Serial.print("PS/clientid "); Serial.println(psCClientID);
    Serial.print("PS/topic "); Serial.println(psCTopic);
    Serial.print("PS/user "); Serial.println(psCUser);
    Serial.print("PS/password "); Serial.println("***");
    Serial.print("PS/domain "); Serial.println(psCDomain);
    Serial.print("PS/port "); Serial.println(psCPort);
    Serial.print("PS/on "); Serial.println((psCConf >> 3) & 1 ? "true" : "false");
    Serial.print("PS/isdomain "); Serial.println((psCConf >> 2) & 1 ? "true" : "false");
    Serial.print("PS/usessl "); Serial.println((psCConf >> 0) & 1 ? "true" : "false");
    Serial.print("PS/auth "); Serial.println((psCConf >> 1) & 1 ? "true" : "false");

    psParamOn.setValue((((psCConf >> 3) & 1) ? "' checked '" : ""), 20);
    psParamIsDomain.setValue((((psCConf >> 2) & 1) ? "' checked '" : ""), 20);
    psParamSSL.setValue((((psCConf >> 0) & 1) ? "' checked '" : ""), 20);
    psParamAuth.setValue((((psCConf >> 1) & 1) ? "' checked '" : ""), 20);

    psPreferences.begin("mqtt");
    psPreferences.putString("domain", psCDomain);
    psPreferences.putString("client", psCClientID);
    psPreferences.putString("user", psCUser);
    psPreferences.putString("password", psCPass);
    psPreferences.putString("topic", psCTopic);
    psPreferences.putUShort("port", psCPort);
    psPreferences.putUChar("config", psCConf);
    psPreferences.end();
}

void loopMqtt() {
    if(useMqtt) {
        // mqtt loop
        connectMqtt();
        psClient.loop();
    }
}
