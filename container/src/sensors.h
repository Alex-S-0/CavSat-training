// Container sensors: pressure, temperature, battery voltage and current.
#pragma once

struct ContainerReadings {
    float pressurePa = 101325.0f;
    float temperatureC = 0;
    float batteryV = 0;
    float batteryMa = 0;
};

namespace sensors {
bool begin();
ContainerReadings read();
}
