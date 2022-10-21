#pragma once

#include <ESP8266WiFi.h>

const struct {
    const String uniqueId = "fan_" + WiFi.macAddress();
    const String name = "greenhouse_fan";
    const String discoveryTopic = "homeassistant/fan/greenhouse/fan/config";
    const String baseTopic = "studiotj/greenhouse/fan";
    const String statusTopic = "studiotj/greenhouse/fan/status";
    const String stateValueTempalte = "{{ value_json.state }}";

    const String commandTopic = "studiotj/greenhouse/fan/power/set";

    const String percentageCommandTopic = "studiotj/greenhouse/fan/percentage/set";

    const String availabilityTemplate = "{{ value_json.availability }}";
    const String percentageValueTemplate = "{{ value_json.percentage }}";
    const int speedPercentageRangeMin = 25;
    const int speedPercentageRangeMax = 100;

} FanInfo;

class Fan {
public:
    Fan();
    uint8_t setPower(boolean enable);
    void handleSetPowerMsg(byte *payload, int length);
    uint8_t setSpeedPercentage(uint8_t percentage);
    void handleSetSpeedPercentage(byte *payload, int length);
    void publishCurrentState();
    static void publishInitialState();
private:
    String state = "OFF";
    uint8_t percentage = FanInfo.speedPercentageRangeMin - 1;
};