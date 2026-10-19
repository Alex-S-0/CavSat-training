// PocketQube pin map. TODO: agree with Electronics and fill in.
#pragma once
#include <stdint.h>

namespace pins {
constexpr uint8_t I2C_SDA = 8;
constexpr uint8_t I2C_SCL = 9;
constexpr uint8_t SD_CS = 10;
constexpr uint8_t GNSS_RX = 17;
constexpr uint8_t GNSS_TX = 18;
constexpr uint8_t PANEL1_RELEASE = 4;
constexpr uint8_t PANEL2_RELEASE = 5;
constexpr uint8_t BOOM_RELEASE = 6;
constexpr uint8_t PANEL1_DETECT = 11;   // PQ32
constexpr uint8_t PANEL2_DETECT = 12;   // PQ32
constexpr uint8_t BOOM_DETECT = 13;     // PQ33
constexpr uint8_t CAMERA_SERVO = 14;    // camera rotation about the nadir axis
constexpr uint8_t SOLAR1_ADC = 1;       // PQ31
constexpr uint8_t SOLAR2_ADC = 2;
}
