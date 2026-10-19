// Container mechanisms and their detectors.
#pragma once
#include <stdint.h>

namespace mechanisms {
void begin();
void fireRelease();        // open the PocketQube release (CTR10)
void resetRelease();       // MEC,REL,OFF for ground testing
uint8_t mechState();       // MECH_STATE bits from the two sensors (CTR11, CTR12)
void cameraRecord(bool on);
}
