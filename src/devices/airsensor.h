#pragma once

#include <ESP8266WiFi.h>

#include "hdc1080.h"

const struct {
    const String uniqueId = "airsensor_temperature_" + WiFi.macAddress();
    const String name = "greenhouse_temperature";
    const String discoveryTopic = "homeassistant/sensor/greenhouse/temperature/config";
    const String baseTopic = "studiotj/greenhouse/temperature";
    const String statusTopic = "studiotj/greenhouse/temperature/status";
    const String availabilityTemplate = "{{ value_json.availability }}";
    const String deviceClass = "temperature";
    const String stateClass = "measurement";
    const String unitOfMeasurement = "°C";
    const String valueTemplate = "{{ value_json.temperature }}";

} TemperatureSensorInfo;

class AirSensor {
public:
    static void publishConfig();
    static void readAndPublish();
private:
    static HDC1080 temperatureSensor;
};