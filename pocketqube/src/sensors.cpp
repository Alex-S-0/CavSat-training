#include "sensors.h"
#include "pins.h"
#include <Arduino.h>
#include <Wire.h>

namespace sensors {

// Hard- and soft-iron calibration for the boom magnetometer.
// TODO: fill from a calibration run (rotate the PocketQube through all axes).
const float MAG_OFFSET[3] = {0, 0, 0};
const float MAG_SCALE[3] = {1, 1, 1};

bool begin() {
    Wire.begin(pins::I2C_SDA, pins::I2C_SCL);
    Serial1.begin(9600, SERIAL_8N1, pins::GNSS_RX, pins::GNSS_TX);
    // TODO: start barometer, IMU, magnetometer, current sensor, GNSS.
    return true;
}

PocketQubeReadings read() {
    PocketQubeReadings r;
    // TODO: real driver reads; rotate each axis into the guide's frame.
    for (int i = 0; i < 3; ++i)
        r.magMilliGauss[i] = (r.magMilliGauss[i] - MAG_OFFSET[i]) * MAG_SCALE[i];
    // TODO: check the solar divider ratio with Electronics.
    r.solar1V = analogReadMilliVolts(pins::SOLAR1_ADC) / 1000.0f;
    r.solar2V = analogReadMilliVolts(pins::SOLAR2_ADC) / 1000.0f;
    return r;
}

} // namespace sensors
