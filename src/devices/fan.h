#pragma once

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
} FanInfo;

class Fan {
public:
    static void publishInitialState();
private:

};