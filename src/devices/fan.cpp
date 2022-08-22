#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include "common.h"
#include "fan.h"
#include "../communication/mqtt.h"

constexpr uint16_t MSG_SIZE = 2048;

void Fan::publishInitialState() {
    StaticJsonDocument<MSG_SIZE> configInfo;
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

    configInfo["state_topic"] = FanInfo.statusTopic;
    configInfo["state_value_template"] = FanInfo.stateValueTempalte;
    // serializeJson(configInfo, Serial);
    // return;

    String outJson;
    serializeJson(configInfo, outJson);
    MQTT::client.publish(FanInfo.discoveryTopic.c_str(), outJson.c_str(), true);

    StaticJsonDocument<512> stateInfo;
    stateInfo["availability"] = Availability.available;
    stateInfo["percentage"] = 50.0f;
    stateInfo["state"] = "OFF";
    outJson.clear();
    serializeJson(stateInfo, outJson);
    MQTT::client.publish(FanInfo.statusTopic.c_str(), outJson.c_str(), true);
}