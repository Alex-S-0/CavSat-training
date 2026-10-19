// Pressure to altitude, plus a simple vertical-speed estimate.
#pragma once
#include <stdint.h>

namespace cavsat {

// Altitude in metres above the reference pressure (international barometric formula).
float pressureToAltitude(float pressurePa, float referencePa);

// Vertical speed from successive altitude samples, smoothed with an EMA.
class VerticalSpeed {
public:
    explicit VerticalSpeed(float alpha = 0.3f) : alpha_(alpha) {}
    float update(float altitudeM, uint32_t nowMs);
    float value() const { return speed_; }
    void reset() { hasLast_ = false; speed_ = 0; }
private:
    float alpha_;
    float speed_ = 0;
    float lastAlt_ = 0;
    uint32_t lastMs_ = 0;
    bool hasLast_ = false;
};

} // namespace cavsat
