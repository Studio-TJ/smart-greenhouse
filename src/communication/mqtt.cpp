#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

#include "mqtt.h"
#include "../devices/common.h"
#include "../devices/fan.h"

constexpr uint16_t CONFIG_MSG_SIZE = 2048;

WiFiClient MQTT::wifiClient = WiFiClient();
PubSubClient MQTT::client = PubSubClient(MQTT::wifiClient);

String MQTT::ip = "";
uint16_t MQTT::port = 1883;
String MQTT::uName = "";
String MQTT::pass = "";

const struct {
    const String uniqueId = "heater_" + WiFi.macAddress();
    const String name = "greenhouse_heater";
    const String baseTopic = "studiotj/greenhouse/heater";
    const String statusTopic = "studiotj/greenhouse/heater/status";
    const struct {
        const String off = "off";
        const String heating = "heating";
        const String idle = "idle";
        const String fan = "fan";
    } actions;
    const struct {
        const String off = "off";
        const String heat = "heat";
        const String fanOnly = "fan_only";
        const String setTopic = "studiotj/greenhouse/heater/mode/set";
    } modes;
    const struct {
        const String automatic = "auto";
        const String low = "low";
        const String medium = "medium";
        const String high = "high";
        const String setTopic = "studiotj/greenhouse/heater/fan_mode/set";
    } fanModes;
    const struct  {
        const float maxTemp = 30;
        const float minTemp = 20;
        const float tempStep = 0.5f;
        const String setTopic = "studiotj/greenhouse/heater/temp_setpoint/set";
    } temperature;
} Heater;

void publishConfigMessage() {
    StaticJsonDocument<CONFIG_MSG_SIZE> configInfo;
    constructDeviceInfo(&configInfo);

    // General config
    configInfo["unique_id"] = Heater.uniqueId;
    configInfo["name"] = Heater.name;
    configInfo["json_attributes_topic"] = Heater.baseTopic;
    // Availability
    configInfo["availability_topic"] = Heater.statusTopic;
    configInfo["availability_template"] = "{{ value_json.availability }}";
    // Action
    configInfo["action_topic"] = Heater.statusTopic;
    configInfo["action_template"] = "{{ value_json.action }}";
    // Modes
    JsonArray modes = configInfo.createNestedArray("modes");
    modes.add(Heater.modes.off);
    modes.add(Heater.modes.heat);
    modes.add(Heater.modes.fanOnly);
    configInfo["mode_state_topic"] = Heater.statusTopic;
    configInfo["mode_state_template"] = "{{ value_json.mode }}";
    configInfo["mode_command_topic"] = Heater.modes.setTopic;
    configInfo["mode_command_template"] = R"({ "mode": "{{ value }}" })";

    // Fan modes
    configInfo["fan_mode_command_topic"] = Heater.fanModes.setTopic;
    configInfo["fan_mode_command_template"] = R"({ "fan_mode": "{{ value }}" })";
    configInfo["fan_mode_state_topic"] = Heater.statusTopic;
    configInfo["fan_mode_state_template"] = "{{ value_json.fan_mode }}";
    JsonArray fanModes = configInfo.createNestedArray("fan_modes");
    fanModes.add(Heater.fanModes.automatic);
    fanModes.add(Heater.fanModes.low);
    fanModes.add(Heater.fanModes.medium);
    fanModes.add(Heater.fanModes.high);

    // Temp
    configInfo["max_temp"] = Heater.temperature.maxTemp;
    configInfo["min_temp"] = Heater.temperature.minTemp;
    configInfo["temp_step"] = Heater.temperature.tempStep;
    configInfo["temperature_command_topic"] = Heater.temperature.setTopic;
    configInfo["temperature_command_template"] = R"({ "tempSetPoint": {{ value }} })";
    configInfo["temperature_state_topic"] = Heater.statusTopic;
    configInfo["temperature_state_template"] = "{{ value_json.tempSetPoint }}";

    // serializeJson(configInfo, Serial);
    // return;
    String outJson;
    serializeJson(configInfo, outJson);
    MQTT::client.setBufferSize(CONFIG_MSG_SIZE);
    MQTT::client.publish("homeassistant/climate/greenhouse/heater/config", outJson.c_str(), true);
}

void publishInitialStateHeater() {
    StaticJsonDocument<512> testJson;
    testJson["action"] = Heater.actions.off;
    testJson["availability"] = Availability.available;
    testJson["mode"] = Heater.modes.off;
    testJson["fan_mode"] = Heater.fanModes.automatic;
    testJson["tempSetPoint"] = 30.0f;
    // serializeJson(testJson, Serial);
    // return;
    String outJson;
    serializeJson(testJson, outJson);
    MQTT::client.publish(Heater.statusTopic.c_str(), outJson.c_str(), true);
}

void MQTT::mqtt_cb(char* topic, byte* payload, unsigned int length) {
    Serial.println(topic);
    if (FanInfo.percentageCommandTopic.equals(topic)) {
        char percentageChar[length] = {0,};
        memcpy(percentageChar, payload, length);
        float percentage = atof(percentageChar);
        Serial.println(percentage);
        int dutyCycle = (int) (percentage * 256 / 100);
        Serial.println(dutyCycle);
        analogWrite(15, dutyCycle);
        // analogWrite(14, dutyCycle);
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

void MQTT::connect(String brokerIp, uint16_t brokerPort, String brokerUserName, String brokerPass) {
    MQTT::ip = brokerIp;
    MQTT::port = brokerPort;
    MQTT::uName = brokerUserName;
    MQTT::pass = brokerPass;
    MQTT::client.setServer(MQTT::ip.c_str(), MQTT::port);
    MQTT::client.setKeepAlive(10);
    if (MQTT::client.connect("Greenhouse Controller", MQTT::uName.c_str(), MQTT::pass.c_str())) {

        publishConfigMessage();
        publishInitialStateHeater();
        Fan::publishInitialState();
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
    }
    else
    {
        ESP.restart();
    }
}