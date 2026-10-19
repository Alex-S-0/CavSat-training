#include "sensors.h"
#include "pins.h"
#include <Wire.h>

namespace sensors {

bool begin() {
    Wire.begin(pins::I2C_SDA, pins::I2C_SCL);
    // TODO: start the barometer (1 Pa resolution, CTR18), temperature (0.1 C, CTR20)
    // and current/voltage monitor (0.01 V, 1 mA, CTR21-22).
    return true;
}

ContainerReadings read() {
    ContainerReadings r;
    // TODO: replace placeholders with real driver reads.
    return r;
}

} // namespace sensors
