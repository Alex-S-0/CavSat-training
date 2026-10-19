// Container pin map. TODO: agree with Electronics and fill in.
#pragma once
#include <stdint.h>

namespace pins {
constexpr uint8_t I2C_SDA = 8;
constexpr uint8_t I2C_SCL = 9;
constexpr uint8_t SD_CS = 10;
constexpr uint8_t RELEASE_SERVO = 4;      // PocketQube release mechanism (CTR10)
constexpr uint8_t RELEASE_SENSOR = 5;     // mechanism fully deployed (CTR11)
constexpr uint8_t RAIL_CLEAR_SENSOR = 6;  // PocketQube cleared the rail (CTR12)
constexpr uint8_t CAMERA_TRIGGER = 7;     // release camera (CTR13)
}
