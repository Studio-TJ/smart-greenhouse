#pragma once

#include <ESP8266WiFi.h>

#include "hdc1080.h"

class AirSensor {
public:
    static void publishConfig();
    static void readAndPublish();
private:
    static HDC1080 temperatureSensor;

    static void publishTemperatureSensorConfig();
    static void publishHumiditySensorConfig();
};