// Keeps the nadir camera pointing north (-Y) within +/-10 deg (C8, PQ15),
// plus the commanded offset (C11, PQ17, PQ18).
#pragma once

namespace camera_stab {
void begin();
// heading: PocketQube heading in degrees from north, from the magnetometer and gyro.
// offsetDeg: commanded rotation. dt: seconds since the last call.
void update(float headingDeg, float yawRateDps, float offsetDeg, float dt);
float lastErrorDeg();
float lastCommandDeg();
void startRecording();
bool recording();
}
