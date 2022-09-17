#include <ArduinoJson.h>

#include "airsensor.h"
#include "common.h"
#include "../communication/mqtt.h"

constexpr uint16_t MSG_SIZE = 2048;
const String statusTopic = "studiotj/greenhouse/air_data/status";
const String availabilityTemplate = "{{ value_json.availability }}";

const struct {
    const String uniqueId = "airsensor_temperature_" + WiFi.macAddress();
    const String name = "greenhouse_temperature";
    const String discoveryTopic = "homeassistant/sensor/greenhouse/temperature/config";
    const String baseTopic = "studiotj/greenhouse/air_data";
    const String deviceClass = "temperature";
    const String stateClass = "measurement";
    const String unitOfMeasurement = "°C";
    const String valueTemplate = "{{ value_json.temperature }}";

} TemperatureSensorInfo;

const struct {
    const String uniqueId = "airsensor_humidity_" + WiFi.macAddress();
    const String name = "greenhouse_humidity";
    const String discoveryTopic = "homeassistant/sensor/greenhouse/humidity/config";
    const String deviceClass = "humidity";
    const String stateClass = "measurement";
    const String unitOfMeasurement = "%";
    const String valueTemplate = "{{ value_json.humidity }}";

} HumiditySensorInfo;

HDC1080 AirSensor::temperatureSensor = HDC1080();

void AirSensor::publishTemperatureSensorConfig() {
    StaticJsonDocument<MSG_SIZE> configInfo;
    constructDeviceInfo(&configInfo);
    configInfo["unique_id"] = TemperatureSensorInfo.uniqueId;
    configInfo["name"] = TemperatureSensorInfo.name;
    configInfo["json_attributes_topic"] = statusTopic;

    configInfo["availability_topic"] = statusTopic;
    configInfo["availability_template"] = availabilityTemplate;

    configInfo["device_class"] = TemperatureSensorInfo.deviceClass;
    configInfo["state_class"] = TemperatureSensorInfo.stateClass;
    configInfo["state_topic"] = statusTopic;
    configInfo["unit_of_measurement"] = TemperatureSensorInfo.unitOfMeasurement;
    configInfo["value_template"] = TemperatureSensorInfo.valueTemplate;

    String outJson;
    serializeJson(configInfo, outJson);

    MQTT::client.publish(TemperatureSensorInfo.discoveryTopic.c_str(), outJson.c_str(), true);
}

void AirSensor::publishHumiditySensorConfig() {
    StaticJsonDocument<MSG_SIZE> configInfo;
    constructDeviceInfo(&configInfo);
    configInfo["unique_id"] = HumiditySensorInfo.uniqueId;
    configInfo["name"] = HumiditySensorInfo.name;
    configInfo["json_attributes_topic"] = statusTopic;

    configInfo["availability_topic"] = statusTopic;
    configInfo["availability_template"] = availabilityTemplate;

    configInfo["device_class"] = HumiditySensorInfo.deviceClass;
    configInfo["state_class"] = HumiditySensorInfo.stateClass;
    configInfo["state_topic"] = statusTopic;
    configInfo["unit_of_measurement"] = HumiditySensorInfo.unitOfMeasurement;
    configInfo["value_template"] = HumiditySensorInfo.valueTemplate;

    String outJson;
    serializeJson(configInfo, outJson);

    MQTT::client.publish(HumiditySensorInfo.discoveryTopic.c_str(), outJson.c_str(), true);
}

void AirSensor::publishConfig() {
    publishTemperatureSensorConfig();
    publishHumiditySensorConfig();

    readAndPublish();
}

void AirSensor::readAndPublish() {
    StaticJsonDocument<512> stateInfo;
    AirData airData = temperatureSensor.measureTempAndHum();
    int temperatureInt = (int)(airData.temperature * 100);
    int humidityInt = (int)(airData.humidity * 100);
    Serial.print("Temp: ");
    Serial.print((double)temperatureInt / 100);
    Serial.print(", hum: ");
    Serial.println(humidityInt);
    if (airData.temperature != -40 ) stateInfo["temperature"] = (double)temperatureInt / 100;
    if (humidityInt != 100) stateInfo["humidity"] = humidityInt;
    stateInfo["availability"] = temperatureSensor.deviceIsAvailable()? Availability.available : Availability.notAvailable;
    String outJson;
    serializeJson(stateInfo, outJson);
    MQTT::client.publish(statusTopic.c_str(), outJson.c_str(), true);
}