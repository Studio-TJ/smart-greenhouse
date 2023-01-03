#pragma once

#include <ESP8266WiFi.h>
#include <PID_v1.h>


const struct {
    const String uniqueId = "heater_" + WiFi.macAddress();
    const String name = "greenhouse_heater";
    const String discoveryTopic = "homeassistant/climate/greenhouse/heater/config";
    const String baseTopic = "studiotj/greenhouse/heater";
    const String statusTopic = "studiotj/greenhouse/heater/status";
    const struct {
        const String off = "off";
        const String heating = "heating";
        const String idle = "idle";
        const String fan = "fan";
    } actions;
    const struct {
        const String off = "off";
        const String heat = "heat";
        const String fanOnly = "fan_only";
        const String setTopic = "studiotj/greenhouse/heater/mode/set";
    } modes;
    const struct {
        const String automatic = "auto";
        const String low = "low";
        const String medium = "medium";
        const String high = "high";
        const String setTopic = "studiotj/greenhouse/heater/fan_mode/set";
    } fanModes;
    const struct  {
        const float maxTemp = 30;
        const float minTemp = 20;
        const float tempStep = 0.5f;
        const String setTopic = "studiotj/greenhouse/heater/temp_setpoint/set";
    } temperature;
    String currentTemperatureTopic = "studiotj/greenhouse/air_data/status";
    String current_temperature_template = "{{ value_json.temperature }}";
} HeaterInfo;

const struct {
    const String outSensorId = "heaterSensor_" + WiFi.macAddress();
    const String name = "greenhouse_heater_heating_level";
    const String baseTopic = "studiotj/greenhouse/heating_level";
    const String statusTopic = "studiotj/greenhouse/heater/status";
    const String discoveryTopic = "homeassistant/sensor/greenhouse/heater_level/config";
    const String stateClass = "measurement";
    const String unitOfMeasurement = "%";
    const String valueTemplate = "{{ value_json.heating_level }}";

} HeaterSensorInfo;

class Heater
{
public:
    Heater();
    uint8_t setPower(boolean enable);
    uint8_t setOutputPercentage(uint8_t percentage);
    void handleSetModeMsg(byte *payload, int length);
    void handleSetFanModeMsg(byte *payload, int length);
    void handleSetTemperatureMsg(byte *payload, int length);
    void pidTick(float currentTemp);

    static void publishInitialState();
    void publishCurrentState();

private:
    String mode = HeaterInfo.modes.off;
    String action = HeaterInfo.actions.off;
    double tempSetpoint = 24.0f;
    double kp;
    double ki;
    double kd;
    PID pid;
    double tempIn = 0;
    double heaterOutput = 0;
    int heaterOutputPercentage = 0;
};