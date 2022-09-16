#pragma once

#include <stdint.h>

/**
 * @brief Defines types of command.
 *
 */
typedef enum {
    SET_FAN = 1 | 1 << 7,
    GET_FAN = 1,

    UNKNOWN = 0,
} CommandType;

typedef struct {
    boolean enable;
    uint8_t percentage : 7;
} FanControl;

typedef union {
    FanControl fanSpeed;
    uint8_t rawValue;
} CommandValue;

inline boolean isSetCommand(CommandType commandType) {
    return commandType & (1 << 7);
}

