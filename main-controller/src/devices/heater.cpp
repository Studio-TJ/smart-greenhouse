#include <ArduinoJson.h>
#include <Wire.h>

#include "../../interface/boardcomm.h"
#include "heater.h"
#include "common.h"
#include "../communication/mqtt.h"

constexpr uint16_t CONFIG_MSG_SIZE = 2048;
constexpr uint16_t STATUS_MSG_SIZE = 256;

Heater::Heater()
    : kp(700)
    , ki(5.95)
    , kd(63000)
    , pid(&tempIn, &heaterOutput, &tempSetpoint, kp, ki, kd, DIRECT) {
    MQTT::callback.emplace(HeaterInfo.modes.setTopic, std::bind(&Heater::handleSetModeMsg, this,
    std::placeholders::_1, std::placeholders::_2));
    // MQTT::callback.emplace(HeaterInfo.modes.setTopic, std::bind(&Heater::handleSetFanModeMsg, this,
    // std::placeholders::_1, std::placeholders::_2));
    MQTT::callback.emplace(HeaterInfo.temperature.setTopic, std::bind(&Heater::handleSetTemperatureMsg, this,
    std::placeholders::_1, std::placeholders::_2));
    Heater::publishInitialState();
    pid.SetMode(AUTOMATIC);
    pid.SetSampleTime(12000);
}

uint8_t Heater::setPower(boolean enable) {
    CommandValue heaterControl = {
        .heaterControl = {
            .enable = enable
        }
    };
    Wire.beginTransmission(AVR_ADDR);
    Wire.write(CommandType::SET_HEATER_POWER);
    Wire.write(heaterControl.rawValue);
    return Wire.endTransmission();
}

uint8_t Heater::setOutputPercentage(uint8_t outputPercentage) {
    CommandValue heaterControl = {
        .heaterControl = {
            .powerPercentage = outputPercentage
        }
    };
    Wire.beginTransmission(AVR_ADDR);
    Wire.write(CommandType::SET_HEATER_OUTPUT);
    Wire.write(heaterControl.rawValue);
    return Wire.endTransmission();
}

void Heater::handleSetModeMsg(byte *payload, int length) {
    char chars[length + 1];
    memcpy(chars, payload, length);
    chars[length] = '\0';
    String modeLocal = String(chars);
    boolean enable = (modeLocal.equals("heat"));
    if (setPower(enable) == 0) {
        mode = modeLocal;
        action = enable ? HeaterInfo.actions.idle : HeaterInfo.actions.off;
        StaticJsonDocument<STATUS_MSG_SIZE> json;
        json["availability"] = Availability.available;
        json["mode"] = mode;
        json["action"] = action;
        String outJson;
        serializeJson(json, outJson);
        MQTT::client.publish(HeaterInfo.statusTopic.c_str(), outJson.c_str(), true);
    }
}

void Heater::handleSetTemperatureMsg(byte *payload, int length) {
    char chars[length + 1];
    memcpy(chars, payload, length);
    chars[length] = '\0';
    tempSetpoint = atof(chars);
    StaticJsonDocument<STATUS_MSG_SIZE> json;
    json["availability"] = Availability.available;
    json["tempSetPoint"] = tempSetpoint;
    String outJson;
    serializeJson(json, outJson);
    MQTT::client.publish(HeaterInfo.statusTopic.c_str(), outJson.c_str(), true);
}

void Heater::pidTick(float currentTemp) {
    if (mode != HeaterInfo.modes.heat) return;
    tempIn = currentTemp;
    pid.Compute();
    if (setOutputPercentage((uint8_t)heaterOutput) == 0) {
        StaticJsonDocument<STATUS_MSG_SIZE> json;
        json["availability"] = Availability.available;
        heaterOutputPercentage = (int)(((double)heaterOutput / 255.0f) * 100);
        if (heaterOutput > 0) {
            action = HeaterInfo.actions.heating;
        } else {
            action = HeaterInfo.actions.idle;
        }
        json["action"] = action;
        json["heating_level"] = heaterOutputPercentage;
        String outJson;
        serializeJson(json, outJson);
        MQTT::client.publish(HeaterInfo.statusTopic.c_str(), outJson.c_str(), true);
    }
    Serial.println(heaterOutput);
}

void Heater::publishCurrentState() {
    StaticJsonDocument<STATUS_MSG_SIZE> stateInfo;
    stateInfo["action"] = action;
    stateInfo["availability"] = Availability.available;
    stateInfo["mode"] = mode;
    stateInfo["fan_mode"] = HeaterInfo.fanModes.automatic;
    stateInfo["tempSetPoint"] = tempSetpoint;
    stateInfo["heating_level"] = heaterOutputPercentage;

    String outJson;
    serializeJson(stateInfo, outJson);
    MQTT::client.publish(HeaterInfo.statusTopic.c_str(), outJson.c_str(), true);
}

void Heater::publishInitialState() {
    StaticJsonDocument<CONFIG_MSG_SIZE> configInfo;
    constructDeviceInfo(&configInfo);

    // Following heater config
    // General config
    configInfo["unique_id"] = HeaterInfo.uniqueId;
    configInfo["name"] = HeaterInfo.name;
    configInfo["json_attributes_topic"] = HeaterInfo.baseTopic;
    // Availability
    configInfo["availability_topic"] = HeaterInfo.statusTopic;
    configInfo["availability_template"] = "{{ value_json.availability }}";
    // Action
    configInfo["action_topic"] = HeaterInfo.statusTopic;
    configInfo["action_template"] = "{{ value_json.action }}";
    // Modes
    JsonArray modes = configInfo.createNestedArray("modes");
    modes.add(HeaterInfo.modes.off);
    modes.add(HeaterInfo.modes.heat);
    // modes.add(HeaterInfo.modes.fanOnly);
    configInfo["mode_state_topic"] = HeaterInfo.statusTopic;
    configInfo["mode_state_template"] = "{{ value_json.mode }}";
    configInfo["mode_command_topic"] = HeaterInfo.modes.setTopic;
    // configInfo["mode_command_template"] = R"({ "mode": "{{ value }}" })";

    configInfo["current_temperature_topic"] = HeaterInfo.currentTemperatureTopic;
    configInfo["current_temperature_template"] = HeaterInfo.current_temperature_template;

    // Fan modes
    configInfo["fan_mode_command_topic"] = HeaterInfo.fanModes.setTopic;
    // configInfo["fan_mode_command_template"] = R"({ "fan_mode": "{{ value }}" })";
    configInfo["fan_mode_state_topic"] = HeaterInfo.statusTopic;
    configInfo["fan_mode_state_template"] = "{{ value_json.fan_mode }}";
    JsonArray fanModes = configInfo.createNestedArray("fan_modes");
    fanModes.add(HeaterInfo.fanModes.automatic);
    // fanModes.add(HeaterInfo.fanModes.low);
    // fanModes.add(HeaterInfo.fanModes.medium);
    // fanModes.add(HeaterInfo.fanModes.high);

    // Temp
    configInfo["max_temp"] = HeaterInfo.temperature.maxTemp;
    configInfo["min_temp"] = HeaterInfo.temperature.minTemp;
    configInfo["temp_step"] = HeaterInfo.temperature.tempStep;
    configInfo["temperature_command_topic"] = HeaterInfo.temperature.setTopic;
    // configInfo["temperature_command_template"] = R"({ "tempSetPoint": {{ value }} })";
    configInfo["temperature_state_topic"] = HeaterInfo.statusTopic;
    configInfo["temperature_state_template"] = "{{ value_json.tempSetPoint }}";

    // serializeJson(configInfo, Serial);
    // return;
    String outJson;
    serializeJson(configInfo, outJson);
    MQTT::client.setBufferSize(CONFIG_MSG_SIZE);
    MQTT::client.publish(HeaterInfo.discoveryTopic.c_str(), outJson.c_str(), true);

    StaticJsonDocument<STATUS_MSG_SIZE> stateInfo;
    stateInfo["action"] = HeaterInfo.actions.off;
    stateInfo["availability"] = Availability.available;
    stateInfo["mode"] = HeaterInfo.modes.off;
    stateInfo["fan_mode"] = HeaterInfo.fanModes.automatic;
    stateInfo["tempSetPoint"] = 24.0f;

    outJson.clear();
    serializeJson(stateInfo, outJson);
    MQTT::client.publish(HeaterInfo.statusTopic.c_str(), outJson.c_str(), true);

    // Following heater level sensor config
    configInfo.clear();
    constructDeviceInfo(&configInfo);
    configInfo["unique_id"] = HeaterSensorInfo.outSensorId;
    configInfo["name"] = HeaterSensorInfo.name;
    configInfo["json_attributes_topic"] = HeaterSensorInfo.baseTopic;

    configInfo["availability_topic"] = HeaterSensorInfo.statusTopic;
    configInfo["availability_template"] = "{{ value_json.availability }}";

    configInfo["state_class"] = HeaterSensorInfo.stateClass;
    configInfo["state_topic"] = HeaterSensorInfo.statusTopic;
    configInfo["unit_of_measurement"] = HeaterSensorInfo.unitOfMeasurement;
    configInfo["value_template"] = HeaterSensorInfo.valueTemplate;

    outJson.clear();
    serializeJson(configInfo, outJson);
    MQTT::client.publish(HeaterSensorInfo.discoveryTopic.c_str(), outJson.c_str(), true);

    stateInfo.clear();
    stateInfo["heating_level"] = 0;
    outJson.clear();
    serializeJson(stateInfo, outJson);
    MQTT::client.publish(HeaterInfo.statusTopic.c_str(), outJson.c_str(), true);
}