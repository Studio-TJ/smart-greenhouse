#include <Arduino.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <TaskScheduler.h>

#include "esp8266ota.h"
#include "pca9555.h"
#include "devices/airsensor.h"
#include "communication/mqtt.h"

uint8_t pin = LOW;
uint16_t dutyCycleValue = 0;
uint8_t testPin = 14;
uint8_t pwmPin = 13;
uint8_t encoderPin = D6;
uint8_t encoderSmallPin = D8;
int16_t delta = 4;
int16_t pwmValue = 0;
uint16_t range = 255;
uint64_t lastMills = 0;
uint64_t lastMillsLong = 0;
volatile uint64_t encoderStep = 0;
volatile uint64_t encoderStepsSmall = 0;

WiFiManager *wifiManager = nullptr;
Esp8266OTA *updater = nullptr;
PCA9555 *pca9555 = nullptr;
PCA9685 *pca9685 = nullptr;
Scheduler *scheduler = nullptr;

Task *updaterTask = nullptr;
Task *mqttTickTask = nullptr;

// void IRAM_ATTR pinTriggered();
void IRAM_ATTR encoderTriggered();
void IRAM_ATTR encoderSmallTriggered();

void initializeWifi();
void initializeScheduler();
void updaterTick();
void mqttTick();

void measurementTick();
Task *measurementTask = nullptr;

void encoderTick();
void encoderSmallTick();
Task *encoderTask = nullptr;
// ADC_MODE(ADC_VCC);
ADC_MODE(ADC_TOUT);

void setup() {
    Serial.begin(115200);
    Wire.begin(D2, D1);
    pca9555 = new PCA9555(1, 1, 1);
    pca9555->setPinType(PIN_1_7, INPUT_PIN);

    pca9685 = new PCA9685(0b1000001);
    // Serial.println(ESP.getVcc());
    initializeWifi();
    updater = new Esp8266OTA("esp8266", "password");
    MQTT::connect("10.238.75.62", 1883, "mqtt", "mqtt");
    AirSensor::publishConfig();
    initializeScheduler();
    // put your setup code here, to run once:
    // pinMode(LED_BUILTIN, OUTPUT);
    // pinMode(testPin, OUTPUT);
    pinMode(pwmPin, OUTPUT);
    // pinMode(0, INPUT_PULLUP);
    // pinMode(D6, OUTPUT);
    // digitalWrite(D6, HIGH);
    pinMode(encoderPin, INPUT_PULLUP);
    // pinMode(encoderSmallPin, INPUT_PULLDOWN_16);
    // pinMode(encoderSmallPin, OUTPUT);
    digitalWrite(encoderSmallPin, LOW);
    pinMode(A0, INPUT);
    // digitalWrite(LED_BUILTIN, pin);
    // digitalWrite(testPin, HIGH);
    // analogWrite(testPin, 0);
    analogWrite(pwmPin, 0);
    analogWriteFreq(500);
    analogWriteRange(range);
    // analogWrite(testPin, 255);
    // attachInterrupt(digitalPinToInterrupt(0), pinTriggered, FALLING);
    attachInterrupt(digitalPinToInterrupt(encoderPin), encoderTriggered, FALLING);
    // attachInterrupt(digitalPinToInterrupt(encoderSmallPin), encoderSmallTriggered, FALLING);
}

void loop() {
    scheduler->execute();
    yield();
    // put your main code here, to run repeatedly:
    // digitalWrite(12, pin);
    // pin = (pin == LOW)? HIGH : LOW;
    // analogWrite(pwmPin, dutyCycleValue);
    // dutyCycleValue += delta;
    // if (dutyCycleValue >= 255) delta = -1;
    // if (dutyCycleValue <= 0) delta = 1;
    // delay(50);
    // if (millis() - lastMills >= 1000UL) {
    //     lastMills = millis();

    // }

    // if (millis() - lastMillsLong >= 1000UL) {
    //     int rpm = (encoderStep / 2) * 60;
    //     encoderStep = 0;
    //     Serial.write("rpm: ");
    //     Serial.println(rpm);
    //     lastMillsLong = millis();
    // }
}

void pinTriggered() {
    pwmValue += delta;
    if (pwmValue >= range) delta = -4;
    if (pwmValue <= 0) delta = 4;
    Serial.println(pwmValue);
    analogWrite(testPin, pwmValue);
}

void encoderTriggered() {
    detachInterrupt(encoderPin);
    // encoderStep ++;
    if (pca9555->readPinValue(PIN_1_7)) encoderStep++;
    // Serial.println(pca9555->readPinValue(PIN_1_7));
    // Serial.println(digitalRead(encoderPin));
    attachInterrupt(digitalPinToInterrupt(encoderPin), encoderTriggered, FALLING);
}

void encoderSmallTriggered() {
    encoderStepsSmall ++;
    // Serial.println(encoderStepsSmall);
}

void encoderTick() {
    int rpm = (encoderStep / 2) * 60;
    // int rpmSmall = (encoderStepsSmall / 2) * 60;
    // Serial.println(encoderStepsSmall);
    encoderStep = 0;
    // encoderStepsSmall = 0;
    // Serial.write("rpm1: ");
    Serial.println(rpm);
    // Serial.write(", rpm2: ");
    // Serial.println(rpmSmall);

    // float sensor = ((float)analogRead(A0)) / 1024.0;
    // float r = (sensor * 820000) / (3.3 - sensor);
    // Serial.println(r);
    // Serial.println(hdc1080->measureTemperature());
    // Serial.println(hdc1080->measureHumidity());
    // AirData airdata = hdc1080->measureTempAndHum();
    // Serial.print(airdata.temperature);
    // Serial.print(", ");
    // Serial.println(airdata.humidity);
    // pca9555->setPinType(PIN_1_7, INPUT_PIN);
    // pca9555->setPinType(PinNumber::PIN_1_0, PinType::OUTPUT_PIN);
    // pca9555->togglePin(PIN_1_0);
    // Serial.println(pca9555->readPinValue(PIN_1_7));
}

void measurementTick() {
    AirSensor::readAndPublish();
}

void initializeWifi() {
    wifiManager = new WiFiManager();
    wifiManager->autoConnect("ESP8266 AP", "12345678");
    WiFi.hostname("greenhouse-controller");
}

void initializeScheduler() {
    scheduler = new Scheduler();
    updaterTask = new Task(TASK_MILLISECOND, TASK_FOREVER, updaterTick, scheduler, true, nullptr, nullptr);
    mqttTickTask = new Task(TASK_MILLISECOND, TASK_FOREVER, mqttTick, scheduler, true, nullptr, nullptr);

    encoderTask = new Task(TASK_SECOND, TASK_FOREVER, encoderTick, scheduler, true, nullptr, nullptr);

    measurementTask = new Task(TASK_SECOND * 15, TASK_FOREVER, measurementTick, scheduler, true, nullptr);
}

void updaterTick() {
    updater->handle();
}

void mqttTick() {
    MQTT::client.loop();
}