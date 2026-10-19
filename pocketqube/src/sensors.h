// PocketQube sensors (PQ24-PQ31, PQ34).
#pragma once
#include <stdint.h>

struct PocketQubeReadings {
    float pressurePa = 101325.0f;
    float temperatureC = 0;
    float batteryV = 0;
    float batteryMa = 0;
    float gyroDps[3] = {0, 0, 0};    // +X east, +Y south, +Z nadir when aligned
    float accelMps2[3] = {0, 0, 0};
    float magMilliGauss[3] = {0, 0, 0};  // calibrated
    float solar1V = 0;
    float solar2V = 0;
    char gnssTime[12] = "00:00:00";
    double lat = 0, lon = 0;
    float gnssAltM = 0;
    uint8_t sats = 0;
};

namespace sensors {
bool begin();
PocketQubeReadings read();
}
