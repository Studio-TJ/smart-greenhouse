#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

#include "mqtt.h"
#include "../../interface/boardcomm.h"
#include "../devices/common.h"
#include "../devices/fan.h"
#include "../devices/airsensor.h"

constexpr uint16_t CONFIG_MSG_SIZE = 2048;

WiFiClient MQTT::wifiClient = WiFiClient();
PubSubClient MQTT::client = PubSubClient(MQTT::wifiClient);

String MQTT::ip = "";
uint16_t MQTT::port = 1883;
String MQTT::uName = "";
String MQTT::pass = "";
boolean MQTT::initialized = false;
std::map<String, Callback> MQTT::callback = {};

void MQTT::mqtt_cb(char* topic, byte* payload, unsigned int length) {
    Serial.println(topic);
    String topicStr = String(topic);
    if (MQTT::callback.count(topicStr) != 0) {
        MQTT::callback[topicStr](payload, length);
    }
    // if (Heater.fanModes.setTopic.equals(topic)) {
    //     // Set fanmode
    //     char pl[length] = {0,};
    //     Serial.println(length);
    //     memcpy(pl, payload, length);
    //     Serial.println(pl);
    //     StaticJsonDocument<CONFIG_MSG_SIZE> doc;
    //     DeserializationError error = deserializeJson(doc, pl);
    //     if (error) {
    //         Serial.print(F("deserializeJson() failed: "));
    //         Serial.println(error.f_str());
    //     }
    //     Serial.println((const char*)doc["fan_mode"]);
    // }
}

void MQTT::checkConnection() {
    // Serial.println(MQTT::client.connected());
    if (!MQTT::initialized) return;
    if (!MQTT::client.connected()) {
        Serial.println("MQTT reconnect");
        MQTT::client.connect("Greenhouse Controller", MQTT::uName.c_str(), MQTT::pass.c_str());
        client.subscribe("studiotj/greenhouse/+/+/set/#");
        client.setCallback(mqtt_cb);
    }
}

void MQTT::connect(String brokerIp, uint16_t brokerPort, String brokerUserName, String brokerPass) {
    MQTT::ip = brokerIp;
    MQTT::port = brokerPort;
    MQTT::uName = brokerUserName;
    MQTT::pass = brokerPass;
    MQTT::client.setServer(MQTT::ip.c_str(), MQTT::port);
    MQTT::client.setKeepAlive(10);
    if (MQTT::client.connect("Greenhouse Controller", MQTT::uName.c_str(), MQTT::pass.c_str())) {

        // Fan::publishInitialState();
        client.subscribe("studiotj/greenhouse/+/+/set/#");
        client.setCallback(mqtt_cb);

        // if (client.subscribe(COMMAND_TOPIC))
        // {
        //     client.setCallback(mqtt_cb);
        // }
        // else
        // {
        //     ESP.restart();
        // }
        MQTT::initialized = true;
    }
    else
    {
        ESP.restart();
    }
}