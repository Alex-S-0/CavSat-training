#include "mechanisms.h"
#include "pins.h"
#include "protocol.h"
#include <Arduino.h>

namespace mechanisms {

void begin() {
    pinMode(pins::PANEL1_RELEASE, OUTPUT);
    pinMode(pins::PANEL2_RELEASE, OUTPUT);
    pinMode(pins::BOOM_RELEASE, OUTPUT);
    pinMode(pins::PANEL1_DETECT, INPUT_PULLUP);
    pinMode(pins::PANEL2_DETECT, INPUT_PULLUP);
    pinMode(pins::BOOM_DETECT, INPUT_PULLUP);
}

// TODO: replace with the real release method (servo, burn wire inside the
// structure per M2, etc.) and its on-time.
static void pulse(uint8_t pin) {
    digitalWrite(pin, HIGH);
    delay(500);
    digitalWrite(pin, LOW);
}

void firePanels() {
    pulse(pins::PANEL1_RELEASE);
    pulse(pins::PANEL2_RELEASE);
}

void fireBoom() { pulse(pins::BOOM_RELEASE); }

uint8_t mechState(bool cameraRecording) {
    uint8_t s = 0;
    if (digitalRead(pins::PANEL1_DETECT) == LOW) s |= cavsat::PMECH_PANEL1;
    if (digitalRead(pins::PANEL2_DETECT) == LOW) s |= cavsat::PMECH_PANEL2;
    if (digitalRead(pins::BOOM_DETECT) == LOW) s |= cavsat::PMECH_BOOM;
    if (cameraRecording) s |= cavsat::PMECH_CAMERA_REC;
    return s;
}

} // namespace mechanisms
