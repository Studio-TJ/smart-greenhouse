#include <Arduino.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <TaskScheduler.h>
#include <ESP8266AVRISP.h>

#include "esp8266ota.h"
#include "devices/airsensor.h"
#include "devices/fan.h"
#include "devices/light.h"
#include "devices/heater.h"
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
Scheduler *scheduler = nullptr;
ESP8266AVRISP *avrIsp = nullptr;
Fan *fan = nullptr;
Light *light = nullptr;
Heater *heater = nullptr;

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
void deviceUpdateStateTick();
Task *measurementTask = nullptr;
Task *deviceUpdateStateTask = nullptr;

void encoderSmallTick();
Task *wdtTask = nullptr;
// ADC_MODE(ADC_VCC);
ADC_MODE(ADC_TOUT);

void setup() {
    Serial.begin(115200);
    Wire.begin(D2, D1);
    pinMode(D4, OUTPUT);
    avrIsp = new ESP8266AVRISP(1000, D4);
    avrIsp->setReset(false);
    avrIsp->begin();

    initializeWifi();
    updater = new Esp8266OTA("esp8266", "password", pauseInterrupt);
    MQTT::connect("10.238.75.62", 1883, "mqtt", "mqtt");
    AirSensor::publishConfig();
    fan = new Fan();
    light = new Light();
    heater = new Heater();
    initializeScheduler();
    pinMode(wdtPin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(encoderPin), encoderTriggered, FALLING);

}

void loop() {
    scheduler->execute();
    yield();
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
    attachInterrupt(digitalPinToInterrupt(encoderPin), encoderTriggered, FALLING);
}

void encoderSmallTriggered() {
    encoderStepsSmall ++;
}

void timerISR() {
    digitalWrite(wdtPin, HIGH);
    timer1_detachInterrupt();
}

void pauseInterrupt() {
    detachInterrupt(encoderPin);
}

void measurementTick() {
    AirSensor::readAndPublish();
    heater->pidTick(AirSensor::temp);
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

    measurementTask = new Task(TASK_SECOND * 10, TASK_FOREVER, measurementTick, scheduler, true, nullptr);

    deviceUpdateStateTask = new Task(TASK_MINUTE, TASK_FOREVER, deviceUpdateStateTick, scheduler, true, nullptr, nullptr);

    wdtTask = new Task(TASK_SECOND * 5, TASK_FOREVER, wdtTick, scheduler, true, nullptr, nullptr);
}

void wdtTick() {
    // Serial.println("WDT tick");
    // digitalWrite(wdtPin, LOW);
    // timer1_attachInterrupt(timerISR);
    // timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
    // timer1_write(600000);
}

void deviceUpdateStateTick() {
    light->publishCurrentState();
    heater->publishCurrentState();
    fan->publishCurrentState();
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