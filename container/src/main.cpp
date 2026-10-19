// Container flight software: 4 Hz telemetry from power-on, release at 90% of
// peak, 1 Hz five seconds after release, radio off after landing.
#include <Arduino.h>
#include <WiFi.h>
#include <string.h>
#include "config.h"
#include "container_fsm.h"
#include "logger.h"
#include "mechanisms.h"
#include "persist.h"
#include "pins.h"
#include "protocol.h"
#include "radio.h"
#include "sensors.h"

using namespace cavsat;

namespace {

ContainerPersist state;
Persist<ContainerPersist> store;
ContainerFsm fsm(state);
Radio radio;
Logger logger;

// Simulation mode (F4-F6): ENABLE then ACTIVATE; SIMP values replace the barometer.
bool simEnabled = false;
bool simActive = false;
float simPressurePa = 101325.0f;

char lastEcho[32] = "NONE";
uint32_t lastTelemetryMs = 0;
uint32_t lastSaveMs = 0;

const uint8_t* const ALLOWED[] = {GROUND_MAC};
const Vehicle SELF = Vehicle::Container;

uint32_t missionMs() { return millis() + (uint32_t)state.utcOffsetS * 1000UL; }

void handleCommand(const Command& c, float currentPressurePa) {
    state.commandCount++;
    strncpy(lastEcho, c.echo, sizeof(lastEcho) - 1);

    switch (c.type) {
        case CommandType::SetTime:
            // TODO: parse hh:mm:ss (or take GPS time from the ground) and set
            // utcOffsetS so the mission clock matches UTC within 1 s (F3).
            break;
        case CommandType::Sim:
            if (c.sim == SimArg::Enable) simEnabled = true;
            else if (c.sim == SimArg::Activate && simEnabled) simActive = true;
            else if (c.sim == SimArg::Disable) simEnabled = simActive = false;
            break;
        case CommandType::SimPressure:
            simPressurePa = (float)c.value;
            break;
        case CommandType::Calibrate:
            fsm.calibrate(currentPressurePa);
            break;
        case CommandType::Arm:
            fsm.arm();
            break;
        case CommandType::Mechanism:
            // MEC,REL,ON|OFF  MEC,CAM,ON|OFF  (testing, FRR, emergencies only)
            if (strcmp(c.arg1, "REL") == 0) {
                if (strcmp(c.arg2, "ON") == 0) mechanisms::fireRelease();
                else mechanisms::resetRelease();
            } else if (strcmp(c.arg1, "CAM") == 0) {
                mechanisms::cameraRecord(strcmp(c.arg2, "ON") == 0);
            }
            break;
        case CommandType::Reset:
            store.clear();  // ground use only: wipes saved flight state
            ESP.restart();
            break;
        default:
            break;
    }
    store.save(state);
}

} // namespace

void setup() {
    Serial.begin(115200);
    bool recovered = store.begin("container", state);
    Serial.printf("Container boot, %s\n", recovered ? "recovered saved state" : "fresh start");
    Serial.printf("This MAC: %s\n", WiFi.macAddress().c_str());

    sensors::begin();
    mechanisms::begin();
    logger.begin(pins::SD_CS, "/container.csv");
    radio.begin(GROUND_MAC, ALLOWED, 1);
}

void loop() {
    ContainerReadings r = sensors::read();
    float pressure = simActive ? simPressurePa : r.pressurePa;

    char line[MAX_PACKET + 1];
    while (radio.receive(line, sizeof(line))) {
        Command c;
        if (parseCommand(line, c, &SELF)) handleCommand(c, pressure);
    }

    uint32_t now = missionMs();
    ContainerOutputs out = fsm.update(pressure, now);
    if (out.fireRelease) {
        mechanisms::fireRelease();
        mechanisms::cameraRecord(true);  // TODO: start earlier so the release is on video
        store.save(state);
    }

    if (now - lastTelemetryMs >= out.telemetryPeriodMs) {
        lastTelemetryMs = now;
        ContainerTelemetry t;
        t.missionTimeS = now / 1000.0f;
        t.packetCount = ++state.packetCount;  // F1
        t.commandCount = state.commandCount;
        t.simMode = simActive;
        t.altitudeM = fsm.altitude();
        t.pressurePa = (int32_t)pressure;     // sim value in sim mode
        t.temperatureC = r.temperatureC;
        t.batteryV = r.batteryV;
        t.batteryMa = (int32_t)r.batteryMa;
        t.mechState = mechanisms::mechState();
        t.state = state.state;
        strncpy(t.cmdEcho, lastEcho, sizeof(t.cmdEcho) - 1);

        size_t n = formatContainer(line, sizeof(line), t);
        if (n) {
            if (out.radioOn) radio.send(line, n);
            logger.write(line);
        }
    }

    // Save often enough to recover, not so often that flash wears out.
    if (now - lastSaveMs >= 1000) {
        lastSaveMs = now;
        store.save(state);
    }
    delay(10);
}
