#include <ArduinoJson.h>
#include <Wire.h>

#include "boardcomm.h"
#include "common.h"
#include "fan.h"
#include "../communication/mqtt.h"

constexpr uint16_t CONFIG_MSG_SIZE = 1024;
constexpr uint16_t STATUS_MSG_SIZE = 128;

Fan::Fan() {
    MQTT::callback.emplace(FanInfo.commandTopic, std::bind(&Fan::handleSetPowerMsg, this, std::placeholders::_1, std::placeholders::_2));
    MQTT::callback.emplace(FanInfo.percentageCommandTopic, std::bind(&Fan::handleSetSpeedPercentage, this, std::placeholders::_1, std::placeholders::_2));
    Fan::publishInitialState();
    setPower(false);
    setSpeedPercentage(0);
}

uint8_t Fan::setPower(boolean enable) {
    CommandValue fanControl = {
         .fanControl = {
            .enable = enable
        }
    };
    Wire.beginTransmission(AVR_ADDR);
    Wire.write(CommandType::SET_FAN_POWER);
    Wire.write(fanControl.rawValue);
    return Wire.endTransmission();
}

void Fan::handleSetPowerMsg(byte *payload, int length) {
    boolean enable = (memcmp(payload, "ON", length) == 0);
    if (setPower(enable) == 0) {
        state = enable ? "ON" : "OFF";
        StaticJsonDocument<STATUS_MSG_SIZE> json;
        json["availability"] = Availability.available;
        json["state"] = state;
        String outJson;
        serializeJson(json, outJson);
        MQTT::client.publish(FanInfo.statusTopic.c_str(), outJson.c_str(), true);
    }
}

uint8_t Fan::setSpeedPercentage(uint8_t percentage) {
    CommandValue fanControl = {
        .fanControl = { .speedPercentage = percentage }
    };
    Wire.beginTransmission(AVR_ADDR);
    Wire.write(CommandType::SET_FAN_SPEED);
    Wire.write(fanControl.rawValue);
    return Wire.endTransmission();
}

void Fan::handleSetSpeedPercentage(byte *payload, int length) {
    char percentageChar[length] = {0,};
    memcpy(percentageChar, payload, length);
    uint8_t percentage = atoi(percentageChar);
    uint8_t percentageMsg = percentage;
    if (percentage == FanInfo.speedPercentageRangeMin - 1) percentage = 0; // speed starts from FanInfo.min min - 1 means 0, it's a limitation of mqtt fan in hassio
    if (setSpeedPercentage(percentage) == 0) {
        this->percentage = percentageMsg;
        StaticJsonDocument<STATUS_MSG_SIZE> json;
        json["availability"] = Availability.available;
        json["percentage"] = this->percentage;
        String outJson;
        serializeJson(json, outJson);
        MQTT::client.publish(FanInfo.statusTopic.c_str(), outJson.c_str(), true);
    }
}

void Fan::publishCurrentState() {
    StaticJsonDocument<STATUS_MSG_SIZE> stateInfo;
    stateInfo["availability"] = Availability.available;
    stateInfo["percentage"] = percentage;
    stateInfo["state"] = state;
    String outJson;
    serializeJson(stateInfo, outJson);
    MQTT::client.publish(FanInfo.statusTopic.c_str(), outJson.c_str(), true);
}

void Fan::publishInitialState() {
    StaticJsonDocument<CONFIG_MSG_SIZE> configInfo;
    constructDeviceInfo(&configInfo);
    configInfo["unique_id"] = FanInfo.uniqueId;
    configInfo["name"] = FanInfo.name;
    configInfo["json_attributes_topic"] = FanInfo.baseTopic;
    configInfo["command_topic"] = FanInfo.commandTopic;
    // Availability
    configInfo["availability_topic"] = FanInfo.statusTopic;
    configInfo["availability_template"] = FanInfo.availabilityTemplate;
    // Percentage
    configInfo["percentage_command_topic"] = FanInfo.percentageCommandTopic;
    configInfo["percentage_state_topic"] = FanInfo.statusTopic;
    configInfo["percentage_value_template"] = FanInfo.percentageValueTemplate;
    configInfo["speed_range_min"] = FanInfo.speedPercentageRangeMin;
    configInfo["speed_range_max"] = FanInfo.speedPercentageRangeMax;

    configInfo["state_topic"] = FanInfo.statusTopic;
    configInfo["state_value_template"] = FanInfo.stateValueTempalte;

    String outJson;
    serializeJson(configInfo, outJson);
    MQTT::client.publish(FanInfo.discoveryTopic.c_str(), outJson.c_str(), true);

    StaticJsonDocument<STATUS_MSG_SIZE> stateInfo;
    stateInfo["availability"] = Availability.available;
    stateInfo["percentage"] = FanInfo.speedPercentageRangeMin - 1;
    stateInfo["state"] = "OFF";
    outJson.clear();
    serializeJson(stateInfo, outJson);
    MQTT::client.publish(FanInfo.statusTopic.c_str(), outJson.c_str(), true);
}