// Mission-wide constants shared by every program.
// Change TEAM_ID and ESPNOW_CHANNEL once the competition assigns them (guide X2).
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace cavsat {

constexpr uint16_t TEAM_ID = 1000;          // TODO: real team number
constexpr uint8_t ESPNOW_CHANNEL = 1;       // TODO: assigned channel (X2)

// MAC of the ground radio. Both vehicles unicast to this peer only (X3, X6).
// TODO: fill in from the ground radio's serial output on first boot.
constexpr uint8_t GROUND_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
// MACs of the vehicles, so the ground radio accepts only our packets (X4).
constexpr uint8_t CONTAINER_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr uint8_t POCKETQUBE_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// ESP-NOW v1 payload limit. A PocketQube CSV line must fit (see docs/ICD.md).
constexpr size_t MAX_PACKET = 250;

// Telemetry rates (CTR15, CTR16, PQ23).
constexpr uint32_t FAST_PERIOD_MS = 250;    // 4 Hz
constexpr uint32_t SLOW_PERIOD_MS = 1000;   // 1 Hz

// Container flight logic thresholds. TODO: tune with Systems.
constexpr float LAUNCH_ALT_M = 20.0f;        // above pad -> ASCENT
constexpr float APOGEE_DESCENT_MPS = -2.0f;  // vertical speed below this...
constexpr uint32_t APOGEE_CONFIRM_MS = 500;  // ...for this long -> APOGEE
constexpr float RELEASE_FRACTION = 0.90f;    // release at 90% of peak (C3)
constexpr uint32_t RATE_DROP_DELAY_MS = 5000; // 1 Hz five seconds after release (CTR16)
constexpr float LANDED_SPEED_MPS = 0.5f;     // |speed| below this...
constexpr uint32_t LANDED_CONFIRM_MS = 5000; // ...for this long -> LANDED
constexpr float LANDED_MAX_ALT_M = 30.0f;    // and below this altitude

// PocketQube timing (C6).
constexpr uint32_t PANEL_DEPLOY_DELAY_MS = 5000;

} // namespace cavsat
