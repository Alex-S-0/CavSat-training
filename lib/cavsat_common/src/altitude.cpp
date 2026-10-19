#include "altitude.h"
#include <math.h>

namespace cavsat {

float pressureToAltitude(float pressurePa, float referencePa) {
    if (pressurePa <= 0 || referencePa <= 0) return 0;
    return 44330.0f * (1.0f - powf(pressurePa / referencePa, 0.190295f));
}

float VerticalSpeed::update(float altitudeM, uint32_t nowMs) {
    if (hasLast_ && nowMs > lastMs_) {
        float raw = (altitudeM - lastAlt_) * 1000.0f / (float)(nowMs - lastMs_);
        speed_ = alpha_ * raw + (1.0f - alpha_) * speed_;
    }
    lastAlt_ = altitudeM;
    lastMs_ = nowMs;
    hasLast_ = true;
    return speed_;
}

} // namespace cavsat
