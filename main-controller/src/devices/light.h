#pragma once

#include <ESP8266WiFi.h>

const struct {
    const String uniqueId = "light_" + WiFi.macAddress();
    const String name = "greenhouse_light";
    const String discoveryTopic = "homeassistant/light/greenhouse/light/config";
    const String baseTopic = "studiotj/greenhouse/light";
    const String statusTopic = "studiotj/greenhouse/light/status";
    const String stateValueTemplate = "{{ value_json.state }}";
    const String commandTopic = "studiotj/greenhouse/light/power/set";
    const String brightnessCommandTopic = "studiotj/greenhouse/light/brightness/set";
    const String brightnessValueTemplate = "{{ value_json.brightness }}";

    const String availabilityTemplate = "{{ value_json.availability }}";
} LightInfo;

class Light {
public:
    Light();
    uint8_t setPower(boolean enable);
    void handleSetPowerMsg(byte *payload, int length);
    uint8_t setBrightness(uint8_t brightnessPercentage);
    void handleSetBrightnessMsg(byte *payload, int length);
    void publishCurrentState();
    static void publishInitialState();

private:
    String state = "OFF";
    uint8_t brightness = 0;
};