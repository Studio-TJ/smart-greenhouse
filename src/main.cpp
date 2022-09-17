#include <Arduino.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <TaskScheduler.h>
#include <Adafruit_PWMServoDriver.h>
#include <ESP8266AVRISP.h>

#include "esp8266ota.h"
#include "pca9555.h"
#include "devices/airsensor.h"
#include "devices/fan.h"
#include "devices/light.h"
#include "communication/mqtt.h"

uint8_t pin = LOW;
uint16_t dutyCycleValue = 0;
uint8_t testPin = 14;
uint8_t wdtPin = 13;
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
Scheduler *scheduler = nullptr;
ESP8266AVRISP *avrIsp = nullptr;
Adafruit_PWMServoDriver *pwmDriver = nullptr;
Fan *fan = nullptr;
Light *light = nullptr;

Task *updaterTask = nullptr;
Task *mqttTickTask = nullptr;
Task *mqttCheckConnectionTask = nullptr;

// void IRAM_ATTR pinTriggered();
void IRAM_ATTR encoderTriggered();
void IRAM_ATTR encoderSmallTriggered();
void IRAM_ATTR timerISR();

void initializeWifi();
void initializeScheduler();
void updaterTick();
void mqttTick();
void mqttCheckConnection();
void wdtTick();

void pauseInterrupt();

void measurementTick();
Task *measurementTask = nullptr;

void encoderTick();
void encoderSmallTick();
Task *encoderTask = nullptr;
Task *wdtTask = nullptr;
// ADC_MODE(ADC_VCC);
ADC_MODE(ADC_TOUT);

void setup() {
    Serial.begin(115200);
    Wire.begin(D2, D1);
    pca9555 = new PCA9555(1, 1, 1);
    pca9555->setPinType(PIN_1_7, INPUT_PIN);
    avrIsp = new ESP8266AVRISP(1000, D4);
    avrIsp->setReset(false);
    avrIsp->begin();

    // pwmDriver = new Adafruit_PWMServoDriver(0b1000001);
    // pwmDriver->begin();
    // pwmDriver->setPWMFreq(500);
    // pwmDriver->setPWM(3, 0, 2048);
    // pwmDriver->setPin(3, 2047);
    // Serial.println(ESP.getVcc());
    initializeWifi();
    updater = new Esp8266OTA("esp8266", "password", pauseInterrupt);
    MQTT::connect("10.238.75.62", 1883, "mqtt", "mqtt");
    AirSensor::publishConfig();
    fan = new Fan();
    light = new Light();
    initializeScheduler();
    // put your setup code here, to run once:
    // pinMode(LED_BUILTIN, OUTPUT);
    // pinMode(testPin, OUTPUT);
    pinMode(wdtPin, OUTPUT);
    // digitalWrite(wdtPin, LOW);
    // pinMode(0, INPUT_PULLUP);
    // pinMode(D6, OUTPUT);
    // digitalWrite(D6, HIGH);
    // pinMode(encoderPin, INPUT_PULLUP);
    // pinMode(encoderSmallPin, INPUT_PULLDOWN_16);
    // pinMode(encoderSmallPin, OUTPUT);
    // digitalWrite(encoderSmallPin, LOW);
    // pinMode(A0, INPUT);
    // digitalWrite(LED_BUILTIN, pin);
    // digitalWrite(testPin, HIGH);
    // analogWrite(testPin, 0);
    // analogWrite(wdtPin, 0);
    // analogWriteFreq(500);
    // analogWriteRange(range);
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
    // analogWrite(wdtPin, dutyCycleValue);
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
    // if (pca9555->readPinValue(PIN_1_7)) encoderStep++;
    // Serial.println(pca9555->readPinValue(PIN_1_7));
    // Serial.println(digitalRead(encoderPin));
    attachInterrupt(digitalPinToInterrupt(encoderPin), encoderTriggered, FALLING);
}

void encoderSmallTriggered() {
    encoderStepsSmall ++;
    // Serial.println(encoderStepsSmall);
}

void timerISR() {
    digitalWrite(wdtPin, HIGH);
    timer1_detachInterrupt();
}

void encoderTick() {
    // Wire.beginTransmission(24);
    // Wire.write((char)20);
    // Wire.endTransmission();
    // Wire.requestFrom(24, 1);
    // MQTT::checkConnection();
    // int rpm = (encoderStep / 2) * 60;
    // int rpmSmall = (encoderStepsSmall / 2) * 60;
    // Serial.println(encoderStepsSmall);
    // encoderStep = 0;
    // encoderStepsSmall = 0;
    // Serial.write("rpm1: ");
    // Serial.println(rpm);
    // Serial.println(pwmDriver->getPWM(3));
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

void pauseInterrupt() {
    detachInterrupt(encoderPin);
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
    mqttCheckConnectionTask = new Task(TASK_SECOND * 30, TASK_FOREVER, mqttCheckConnection, scheduler, true, nullptr, nullptr);

    encoderTask = new Task(TASK_SECOND, TASK_FOREVER, encoderTick, scheduler, true, nullptr, nullptr);

    measurementTask = new Task(TASK_SECOND * 15, TASK_FOREVER, measurementTick, scheduler, true, nullptr);

    wdtTask = new Task(TASK_SECOND * 5, TASK_FOREVER, wdtTick, scheduler, true, nullptr, nullptr);
}

void wdtTick() {
    // Serial.println("WDT tick");
    // digitalWrite(wdtPin, LOW);
    // timer1_attachInterrupt(timerISR);
    // timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
    // timer1_write(600000);
}

void updaterTick() {
    updater->handle();
  static AVRISPState_t last_state = AVRISP_STATE_IDLE;
  AVRISPState_t new_state = avrIsp->update();
  if (last_state != new_state) {
    switch (new_state) {
      case AVRISP_STATE_IDLE:
        {
          Serial.printf("[AVRISP] now idle\r\n");
          // Use the SPI bus for other purposes
          break;
        }
      case AVRISP_STATE_PENDING:
        {
          Serial.printf("[AVRISP] connection pending\r\n");
          // Clean up your other purposes and prepare for programming mode
          break;
        }
      case AVRISP_STATE_ACTIVE:
        {
          Serial.printf("[AVRISP] programming mode\r\n");
          // Stand by for completion
          break;
        }
    }
    last_state = new_state;
  }
  // Serve the client
  if (last_state != AVRISP_STATE_IDLE) { avrIsp->serve(); }
}

void mqttTick() {
    MQTT::client.loop();
}

void mqttCheckConnection() {
    MQTT::checkConnection();
}