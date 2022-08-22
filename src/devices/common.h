#pragma once

#include <ArduinoJson.h>

const struct {
    String available = "online";
    String notAvailable = "offline";
} Availability;

void constructDeviceInfo(JsonDocument *config);