// Container flight logic, kept free of hardware so it runs in host tests.
#pragma once
#include <stdint.h>
#include "altitude.h"
#include "protocol.h"

namespace cavsat {

// Everything that must survive a reset (F1, F2, F8). The firmware saves this
// struct to flash; the state machine only reads and changes it.
struct ContainerPersist {
    uint32_t magic = 0;
    ContainerState state = ContainerState::LaunchPad;
    bool armed = false;
    float referencePa = 101325.0f;  // set by CAL
    float peakAltM = 0;
    uint32_t packetCount = 0;
    uint32_t commandCount = 0;
    bool releaseFired = false;      // never fire twice after a reset
    uint32_t releaseAtMs = 0;       // mission time of release, for the 1 Hz switch
    int32_t utcOffsetS = 0;         // set by ST; mission clock = uptime + offset
};

struct ContainerOutputs {
    bool fireRelease = false;  // caller drives the release mechanism
    bool radioOn = true;
    uint32_t telemetryPeriodMs = 250;
};

class ContainerFsm {
public:
    explicit ContainerFsm(ContainerPersist& p) : p_(p) {}

    // Call every sample with the pressure in use (real or SIMP) and mission ms.
    ContainerOutputs update(float pressurePa, uint32_t missionMs);

    void arm() { p_.armed = true; }
    void calibrate(float padPressurePa) { p_.referencePa = padPressurePa; p_.peakAltM = 0; }

    float altitude() const { return alt_; }
    float speed() const { return vs_.value(); }

private:
    ContainerPersist& p_;
    VerticalSpeed vs_;
    float alt_ = 0;
    uint32_t condSinceMs_ = 0;
    bool condActive_ = false;

    bool held(bool cond, uint32_t nowMs, uint32_t forMs);
};

} // namespace cavsat
