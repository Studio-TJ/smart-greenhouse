#include <Arduino.h>
#include <Wire.h>

#include "boardcomm.h"

double rRef = 61400;
double rMeasure = 0;

void i2cRequest();
void i2cReceive(int nrBytes);

void setFanPower(boolean enable);
void setFanSpeed(uint8_t percentage);
void setLightPower(boolean enable);
void setLightBrightness(uint8_t brightnessPercentage);

// constexpr uint8_t intPin = 7;

constexpr uint8_t fanPwmPin = 5;
constexpr uint8_t fanPowerPin = 4;
constexpr uint8_t heaterPwmPin = 6;
constexpr uint8_t heaterPowerPin = 7;
constexpr uint8_t lightPwmPin = 9;
constexpr uint8_t lightPowerPin = 8;
constexpr uint8_t i2cAddress = 24;

static CommandType currentCommand = CommandType::UNKNOWN;
static CommandValue commandValue = { .rawValue = 0 };
static uint8_t fanSpeedPercentage = 0;
static boolean fanPower = false;
static uint8_t lightBrightnessPercentage = 0;
static boolean lightPower = false;
inline uint8_t percentageToDutyCycle(uint8_t percentage) {
    return (255 * percentage) / 100;
}

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.println("boot");
    pinMode(LED_BUILTIN, OUTPUT);
    // pinMode(intPin, OUTPUT);
    // digitalWrite(intPin, HIGH);
    pinMode(A1, INPUT);
    pinMode(fanPwmPin, OUTPUT);
    pinMode(fanPowerPin, OUTPUT);
    pinMode(lightPowerPin, OUTPUT);
    pinMode(lightPwmPin, OUTPUT);

    analogWrite(fanPwmPin, 0);
    digitalWrite(fanPower, LOW);
    analogWrite(lightPwmPin, 0);
    digitalWrite(lightPowerPin, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    Wire.begin(i2cAddress);
    Wire.onRequest(i2cRequest);
    Wire.onReceive(i2cReceive);
}

void loop() {
    // put your main code here, to run repeatedly:
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    // int analogRaw = analogRead(A1);
    // rMeasure = ((double)analogRaw / (double)1023) * rRef;
    // Serial.print(analogRaw);
    // Serial.print(", ");
    // Serial.println(rMeasure);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}

void i2cRequest() {
    Serial.println("i2c on request");
}

void i2cReceive(int nrBytes) {
    // Should only set command
    Serial.print("i2c on Receive, ");
    currentCommand = (CommandType) Wire.read();
    if (isSetCommand(currentCommand)) {
        commandValue.rawValue = Wire.read();
    }
    switch (currentCommand) {
        case CommandType::SET_FAN_POWER:
            setFanPower(commandValue.fanControl.enable);
            break;
        case CommandType::SET_FAN_SPEED:
            setFanSpeed(commandValue.fanControl.percentage);
            break;
        case CommandType::SET_LIGHT_POWER:
            setLightPower(commandValue.lightControl.enable);
            break;
        case CommandType::SET_LIGHT_BRIGHTNESS:
            setLightBrightness(commandValue.lightControl.brightness);
            break;

        default:
            break;
    }
}

void setFanSpeed(uint8_t percentage) {
    Serial.println(percentage);
    fanSpeedPercentage = percentage;
    analogWrite(fanPwmPin, percentageToDutyCycle(percentage));
}

void setFanPower(boolean enable) {
    fanPower = enable;
    digitalWrite(fanPowerPin, enable ? HIGH : LOW);
}

void setLightPower(boolean enable) {
    lightPower = enable;
    digitalWrite(lightPowerPin, enable ? HIGH : LOW);
}
void setLightBrightness(uint8_t brightness) {
    Serial.println(brightness);
    lightBrightnessPercentage = brightness;
    analogWrite(lightPwmPin, brightness);
}