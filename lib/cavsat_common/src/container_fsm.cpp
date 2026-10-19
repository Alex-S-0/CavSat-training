#include "container_fsm.h"
#include "config.h"

namespace cavsat {

bool ContainerFsm::held(bool cond, uint32_t nowMs, uint32_t forMs) {
    if (!cond) { condActive_ = false; return false; }
    if (!condActive_) { condActive_ = true; condSinceMs_ = nowMs; }
    return nowMs - condSinceMs_ >= forMs;
}

ContainerOutputs ContainerFsm::update(float pressurePa, uint32_t missionMs) {
    ContainerOutputs out;
    alt_ = pressureToAltitude(pressurePa, p_.referencePa);
    float v = vs_.update(alt_, missionMs);
    if (alt_ > p_.peakAltM) p_.peakAltM = alt_;

    switch (p_.state) {
        case ContainerState::LaunchPad:
            // Launch detection only after ARM.
            if (p_.armed && alt_ > LAUNCH_ALT_M) {
                p_.state = ContainerState::Ascent;
                condActive_ = false;
            }
            break;
        case ContainerState::Ascent:
            // Speed only, so the state can never fall back (1093's lesson).
            if (held(v < APOGEE_DESCENT_MPS, missionMs, APOGEE_CONFIRM_MS)) {
                p_.state = ContainerState::Apogee;
                condActive_ = false;
            }
            break;
        case ContainerState::Apogee:
            if (!p_.releaseFired && alt_ <= RELEASE_FRACTION * p_.peakAltM) {
                out.fireRelease = true;
                p_.releaseFired = true;
                p_.releaseAtMs = missionMs;
                p_.state = ContainerState::PqRelease;
            }
            break;
        case ContainerState::PqRelease: {
            bool still = v > -LANDED_SPEED_MPS && v < LANDED_SPEED_MPS;
            if (held(still && alt_ < LANDED_MAX_ALT_M, missionMs, LANDED_CONFIRM_MS)) {
                p_.state = ContainerState::Landed;
            }
            break;
        }
        case ContainerState::Landed:
            break;
    }

    if (p_.releaseFired && missionMs - p_.releaseAtMs >= RATE_DROP_DELAY_MS) {
        out.telemetryPeriodMs = SLOW_PERIOD_MS;  // CTR16
    } else {
        out.telemetryPeriodMs = FAST_PERIOD_MS;  // CTR15
    }
    out.radioOn = p_.state != ContainerState::Landed;  // stop after landing
    return out;
}

} // namespace cavsat
