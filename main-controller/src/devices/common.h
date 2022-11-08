#pragma once

#include <ArduinoJson.h>

constexpr uint8_t AVR_ADDR = 24;

const struct {
    String available = "online";
    String notAvailable = "offline";
} Availability;

void constructDeviceInfo(JsonDocument *config);