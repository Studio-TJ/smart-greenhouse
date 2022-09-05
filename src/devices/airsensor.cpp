#include <ArduinoJson.h>

#include "airsensor.h"
#include "common.h"
#include "../communication/mqtt.h"

constexpr uint16_t MSG_SIZE = 2048;

HDC1080 AirSensor::temperatureSensor = HDC1080();

void AirSensor::publishConfig() {
    StaticJsonDocument<MSG_SIZE> configInfo;
    constructDeviceInfo(&configInfo);
    configInfo["unique_id"] = TemperatureSensorInfo.uniqueId;
    configInfo["name"] = TemperatureSensorInfo.name;
    configInfo["json_attributes_topic"] = TemperatureSensorInfo.statusTopic;

    configInfo["availability_topic"] = TemperatureSensorInfo.statusTopic;
    configInfo["availability_template"] = TemperatureSensorInfo.availabilityTemplate;

    configInfo["device_class"] = TemperatureSensorInfo.deviceClass;
    configInfo["state_class"] = TemperatureSensorInfo.stateClass;
    configInfo["state_topic"] = TemperatureSensorInfo.statusTopic;
    configInfo["unit_of_measurement"] = TemperatureSensorInfo.unitOfMeasurement;
    configInfo["value_template"] = TemperatureSensorInfo.valueTemplate;

    String outJson;
    serializeJson(configInfo, outJson);

    MQTT::client.publish(TemperatureSensorInfo.discoveryTopic.c_str(), outJson.c_str(), true);

    readAndPublish();
}

void AirSensor::readAndPublish() {
    StaticJsonDocument<512> stateInfo;
    int temperature = (int)(temperatureSensor.measureTemperature() * 100);
    stateInfo["temperature"] = (double)temperature / 100;
    stateInfo["availability"] = temperatureSensor.deviceIsAvailable()? Availability.available : Availability.notAvailable;
    String outJson;
    serializeJson(stateInfo, outJson);
    MQTT::client.publish(TemperatureSensorInfo.statusTopic.c_str(), outJson.c_str(), true);
}