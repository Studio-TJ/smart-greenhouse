#pragma once

#include <stdint.h>

/**
 * @brief Defines types of command.
 *
 */
typedef enum {
    SET_FAN_POWER = 1 | 1 << 7,
    GET_FAN_POWER = 1,
    SET_FAN_SPEED = 2 | 1 << 7,
    GET_FAN_SPEED = 2,
    SET_LIGHT_POWER = 3 | 1 << 7,
    GET_LIGHT_POWER = 3,
    SET_LIGHT_BRIGHTNESS = 4 | 1 << 7,
    GET_LIGHT_BRIGHTNESS = 4,
    SET_HEATER_POWER = 5 | 1 << 7,
    GET_HEATER_POWER = 5,
    SET_HEATER_OUTPUT = 6 | 1 << 7,
    GET_HEATER_OUTPUT = 6,

    UNKNOWN = 0,
} CommandType;

typedef union {
    boolean enable;
    uint8_t speedPercentage;
} FanControl;

typedef union {
    boolean enable;
    uint8_t brightness;
} LightControl;

typedef union {
    boolean enable;
    uint8_t powerPercentage;
} HeaterControl;

typedef union {
    FanControl fanControl;
    LightControl lightControl;
    HeaterControl heaterControl;
    uint8_t rawValue;
} CommandValue;

inline boolean isSetCommand(CommandType commandType) {
    return commandType & (1 << 7);
}

