#include <ArduinoJson.h>
#include <Wire.h>

#include "boardcomm.h"
#include "common.h"
#include "light.h"
#include "../communication/mqtt.h"

constexpr uint16_t CONFIG_MSG_SIZE = 1024;
constexpr uint16_t STATUS_MSG_SIZE = 128;

Light::Light() {
    MQTT::callback.emplace(LightInfo.commandTopic, std::bind(&Light::handleSetPowerMsg, this, std::placeholders::_1, std::placeholders::_2));
    MQTT::callback.emplace(LightInfo.brightnessCommandTopic, std::bind(&Light::handleSetBrightnessMsg, this, std::placeholders::_1, std::placeholders::_2));
    Light::publishInitialState();
    setPower(false);
    setBrightness(0);
}

uint8_t Light::setPower(boolean enable) {
    Wire.beginTransmission(AVR_ADDR);
    CommandValue lightControl =  {
        .lightControl = {
            .enable = enable
        }
    };
    Wire.write(CommandType::SET_LIGHT_POWER);
    Wire.write(lightControl.rawValue);
    return Wire.endTransmission();
}

void Light::handleSetPowerMsg(byte *payload, int length) {
    boolean enable = (memcmp(payload, "ON", length) == 0);
    if (setPower(enable) == 0) {
        state = enable ? "ON" : "OFF";
        StaticJsonDocument<STATUS_MSG_SIZE> json;
        json["availability"] = Availability.available;
        json["state"] = state;
        String outJson;
        serializeJson(json, outJson);
        MQTT::client.publish(LightInfo.statusTopic.c_str(), outJson.c_str(), true);
    }
}

uint8_t Light::setBrightness(uint8_t brightnessPercentage) {
    CommandValue lightControl = {
        .lightControl = { .brightness = brightnessPercentage }
    };
    Wire.beginTransmission(AVR_ADDR);
    Wire.write(CommandType::SET_LIGHT_BRIGHTNESS);
    Wire.write(lightControl.rawValue);
    return Wire.endTransmission();
}

void Light::handleSetBrightnessMsg(byte *payload, int length) {
    char brightnessChar[length] = {0,};
    memcpy(brightnessChar, payload, length);
    uint8_t brightness = atoi(brightnessChar);
    if (setBrightness(brightness) == 0) {
        this->brightness = brightness;
        StaticJsonDocument<STATUS_MSG_SIZE> json;
        json["availability"] = Availability.available;
        json["brightness"] = this->brightness;
        String outJson;
        serializeJson(json, outJson);
        MQTT::client.publish(LightInfo.statusTopic.c_str(), outJson.c_str(), true);
    }
}

void Light::publishInitialState() {
    StaticJsonDocument<CONFIG_MSG_SIZE> configInfo;
    constructDeviceInfo(&configInfo);
    configInfo["unique_id"] = LightInfo.uniqueId;
    configInfo["name"] = LightInfo.name;
    configInfo["json_attributes_topic"] = LightInfo.baseTopic;
    configInfo["command_topic"] = LightInfo.commandTopic;
    configInfo["brightness_command_topic"] = LightInfo.brightnessCommandTopic;
    configInfo["brightness_state_topic"] = LightInfo.statusTopic;
    configInfo["brightness_value_template"] = LightInfo.brightnessValueTemplate;
    configInfo["state_topic"] = LightInfo.statusTopic;
    configInfo["state_value_template"] = LightInfo.stateValueTemplate;
    configInfo["availability_topic"] = LightInfo.statusTopic;
    configInfo["availability_template"] = LightInfo.availabilityTemplate;

    String outJson;
    serializeJson(configInfo, outJson);
    MQTT::client.publish(LightInfo.discoveryTopic.c_str(), outJson.c_str(), true);

    StaticJsonDocument<STATUS_MSG_SIZE> stateInfo;
    stateInfo["availability"] = Availability.available;
    stateInfo["state"] = "OFF";
    stateInfo["brightness"] = 0;
    outJson.clear();
    serializeJson(stateInfo, outJson);
    MQTT::client.publish(LightInfo.statusTopic.c_str(), outJson.c_str(), true);
}

void Light::publishCurrentState() {
    StaticJsonDocument<STATUS_MSG_SIZE> stateInfo;
    stateInfo["availability"] = Availability.available;
    stateInfo["state"] = state;
    stateInfo["brightness"] = brightness;
    String outJson;
    serializeJson(stateInfo, outJson);
    MQTT::client.publish(LightInfo.statusTopic.c_str(), outJson.c_str(), true);
}