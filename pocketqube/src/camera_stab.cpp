#include "camera_stab.h"
#include "pins.h"
#include <Arduino.h>

namespace camera_stab {

namespace {
// TODO: tune on the spinning test rig.
float KP = 1.0f, KD = 0.05f;
float error_ = 0, command_ = 0;
bool recording_ = false;

float wrap180(float a) {
    while (a > 180) a -= 360;
    while (a < -180) a += 360;
    return a;
}
} // namespace

void begin() {
    // TODO: attach the camera rotation servo or motor on pins::CAMERA_SERVO.
}

void update(float headingDeg, float yawRateDps, float offsetDeg, float dt) {
    (void)dt;
    // The camera must counter-rotate so its view stays at north + offset.
    error_ = wrap180(offsetDeg - headingDeg);
    command_ = KP * error_ - KD * yawRateDps;
    // TODO: send command_ to the actuator, respecting its travel limits.
}

float lastErrorDeg() { return error_; }
float lastCommandDeg() { return command_; }

void startRecording() {
    recording_ = true;  // TODO: start the 640x480 colour recorder (PQ14, PQ16)
}

bool recording() { return recording_; }

} // namespace camera_stab
