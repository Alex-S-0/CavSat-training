#include "mechanisms.h"
#include "pins.h"
#include "protocol.h"
#include <Arduino.h>

namespace mechanisms {

void begin() {
    pinMode(pins::RELEASE_SENSOR, INPUT_PULLUP);
    pinMode(pins::RAIL_CLEAR_SENSOR, INPUT_PULLUP);
    pinMode(pins::CAMERA_TRIGGER, OUTPUT);
    // TODO: attach the release servo or actuator on pins::RELEASE_SERVO.
}

void fireRelease() {
    // TODO: drive the release actuator to the open position.
}

void resetRelease() {
    // TODO: return the actuator to the stowed position.
}

uint8_t mechState() {
    uint8_t s = 0;
    // TODO: check switch polarity with Mechanisms.
    if (digitalRead(pins::RELEASE_SENSOR) == LOW) s |= cavsat::CMECH_RELEASE_ACTIVATED;
    if (digitalRead(pins::RAIL_CLEAR_SENSOR) == LOW) s |= cavsat::CMECH_PQ_RELEASED;
    return s;
}

void cameraRecord(bool on) {
    digitalWrite(pins::CAMERA_TRIGGER, on ? HIGH : LOW);  // TODO: match the camera's trigger
}

} // namespace mechanisms
