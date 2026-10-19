// PocketQube deployables and their detectors.
#pragma once
#include <stdint.h>

namespace mechanisms {
void begin();
void firePanels();   // both solar panels (C6, PQ8)
void fireBoom();     // magnetometer boom (C9, PQ11)
uint8_t mechState(bool cameraRecording);  // bits 0-3 (PQ32, PQ33)
}
