// PocketQube timing logic: panels at +5 s, nothing re-fires after a reset.
#pragma once
#include <stdint.h>

namespace cavsat {

struct PocketQubePersist {
    uint32_t magic = 0;
    uint32_t bootCount = 0;
    uint32_t packetCount = 0;
    uint32_t commandCount = 0;
    bool panelsFired = false;
    bool boomFired = false;
    float cameraOffsetDeg = 0;   // commanded rotation from north (C11)
    float referencePa = 101325.0f;
};

struct PocketQubeOutputs {
    bool firePanels = false;
};

class PocketQubeFsm {
public:
    explicit PocketQubeFsm(PocketQubePersist& p) : p_(p) {}
    // sincePowerMs: time since the PocketQube powered up on leaving the rail.
    PocketQubeOutputs update(uint32_t sincePowerMs);
    bool requestBoom();                       // false if it already fired
    void setCameraOffset(float deg) { p_.cameraOffsetDeg = deg; }
private:
    PocketQubePersist& p_;
};

} // namespace cavsat
