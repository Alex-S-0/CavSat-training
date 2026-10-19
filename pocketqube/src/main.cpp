// PocketQube flight software: boots when it leaves the rail, transmits at 4 Hz,
// deploys panels at +5 s, deploys the boom and rotates the camera on command.
#include <Arduino.h>
#include <WiFi.h>
#include <math.h>
#include <string.h>
#include "altitude.h"
#include "camera_stab.h"
#include "config.h"
#include "logger.h"
#include "mechanisms.h"
#include "persist.h"
#include "pins.h"
#include "pocketqube_fsm.h"
#include "protocol.h"
#include "radio.h"
#include "sensors.h"

using namespace cavsat;

namespace {

PocketQubePersist state;
Persist<PocketQubePersist> store;
PocketQubeFsm fsm(state);
Radio radio;
Logger logger;

bool simEnabled = false, simActive = false;
float simPressurePa = 101325.0f;
char lastEcho[32] = "NONE";
uint32_t lastTelemetryMs = 0, lastLoopMs = 0, lastSaveMs = 0;

const uint8_t* const ALLOWED[] = {GROUND_MAC};
const Vehicle SELF = Vehicle::PocketQube;

float headingDeg(const PocketQubeReadings& r) {
    // TODO: fuse gyro and magnetometer (complementary filter) instead of raw mag.
    return atan2f(r.magMilliGauss[0], -r.magMilliGauss[1]) * 180.0f / (float)M_PI;
}

void handleCommand(const Command& c, float currentPressurePa) {
    state.commandCount++;
    strncpy(lastEcho, c.echo, sizeof(lastEcho) - 1);
    switch (c.type) {
        case CommandType::Sim:
            if (c.sim == SimArg::Enable) simEnabled = true;
            else if (c.sim == SimArg::Activate && simEnabled) simActive = true;
            else if (c.sim == SimArg::Disable) simEnabled = simActive = false;
            break;
        case CommandType::SimPressure:
            simPressurePa = (float)c.value;
            break;
        case CommandType::Calibrate:
            state.referencePa = currentPressurePa;
            break;
        case CommandType::Mechanism:
            // MEC,BOOM,ON  MEC,PANEL,ON  MEC,CAM,<degrees>  MEC,REC,ON
            if (strcmp(c.arg1, "BOOM") == 0 && fsm.requestBoom()) mechanisms::fireBoom();
            else if (strcmp(c.arg1, "PANEL") == 0) mechanisms::firePanels();
            else if (strcmp(c.arg1, "CAM") == 0) fsm.setCameraOffset((float)c.value);
            else if (strcmp(c.arg1, "REC") == 0) camera_stab::startRecording();
            break;
        case CommandType::Reset:
            store.clear();
            ESP.restart();
            break;
        default:
            break;
    }
    store.save(state);
}

} // namespace

void setup() {
    // Keep this path short: the PocketQube must transmit right after release (C5).
    Serial.begin(115200);
    bool recovered = store.begin("pocketqube", state);
    state.bootCount++;
    radio.begin(GROUND_MAC, ALLOWED, 1);
    camera_stab::startRecording();  // recording starts at power-on (PQ16)
    sensors::begin();
    mechanisms::begin();
    camera_stab::begin();
    logger.begin(pins::SD_CS, "/pocketqube.csv");
    Serial.printf("PocketQube boot %lu, %s, MAC %s\n", (unsigned long)state.bootCount,
                  recovered ? "recovered" : "fresh", WiFi.macAddress().c_str());
}

void loop() {
    uint32_t now = millis();  // TODO: add a saved offset so mission time survives resets (F2)
    PocketQubeReadings r = sensors::read();
    float pressure = simActive ? simPressurePa : r.pressurePa;

    char line[MAX_PACKET + 1];
    while (radio.receive(line, sizeof(line))) {
        Command c;
        if (parseCommand(line, c, &SELF)) handleCommand(c, pressure);
    }

    PocketQubeOutputs out = fsm.update(now);
    if (out.firePanels) {
        mechanisms::firePanels();
        store.save(state);
    }

    float dt = (now - lastLoopMs) / 1000.0f;
    lastLoopMs = now;
    camera_stab::update(headingDeg(r), r.gyroDps[2], state.cameraOffsetDeg, dt);

    if (now - lastTelemetryMs >= FAST_PERIOD_MS) {
        lastTelemetryMs = now;
        PocketQubeTelemetry t;
        t.missionTimeS = now / 1000.0f;
        t.packetCount = ++state.packetCount;
        t.commandCount = state.commandCount;
        t.simMode = simActive;
        t.altitudeM = pressureToAltitude(pressure, state.referencePa);
        t.temperatureC = r.temperatureC;
        t.pressurePa = (int32_t)pressure;
        t.batteryV = r.batteryV;
        t.batteryMa = (int32_t)r.batteryMa;
        strncpy(t.gnssTime, r.gnssTime, sizeof(t.gnssTime) - 1);
        t.gnssAltM = r.gnssAltM;
        t.gnssLat = r.lat;
        t.gnssLon = r.lon;
        t.gnssSats = r.sats;
        for (int i = 0; i < 3; ++i) {
            t.rot[i] = r.gyroDps[i];
            t.accel[i] = r.accelMps2[i];
            t.mag[i] = (int32_t)r.magMilliGauss[i];
        }
        t.solar1V = r.solar1V;
        t.solar2V = r.solar2V;
        t.mechState = mechanisms::mechState(camera_stab::recording());
        strncpy(t.cmdEcho, lastEcho, sizeof(t.cmdEcho) - 1);
        t.stabErrorDeg = camera_stab::lastErrorDeg();
        t.stabCommandDeg = camera_stab::lastCommandDeg();

        size_t n = formatPocketQube(line, sizeof(line), t);
        if (n) {
            radio.send(line, n);
            logger.write(line);
        } else {
            Serial.println("Telemetry line too long for one ESP-NOW frame");
        }
    }

    if (now - lastSaveMs >= 1000) {
        lastSaveMs = now;
        store.save(state);
    }
    delay(5);
}
