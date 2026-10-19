// Host tests for the shared logic.
// Build and run from this folder (no ESP32 needed):
//   g++ -std=c++17 -I../src test_host.cpp ../src/*.cpp -o test_host && ./test_host
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "altitude.h"
#include "config.h"
#include "container_fsm.h"
#include "pocketqube_fsm.h"
#include "protocol.h"

using namespace cavsat;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static float pressureAt(float altM, float ref = 101325.0f) {
    return ref * powf(1.0f - altM / 44330.0f, 1.0f / 0.190295f);
}

static void testCommands() {
    char buf[96];
    formatCommand(buf, sizeof(buf), Vehicle::Container, "SIMP", "101325");
    Command c;
    CHECK(parseCommand(buf, c));
    CHECK(c.type == CommandType::SimPressure && c.value == 101325);
    CHECK(strcmp(c.echo, "SIMP101325") == 0);

    Vehicle pq = Vehicle::PocketQube;
    CHECK(!parseCommand(buf, c, &pq));             // addressed to the container
    CHECK(!parseCommand("CMD,9999,C,CAL", c));     // another team
    CHECK(parseCommand("CMD,1000,P,MEC,CAM,90\n", c, &pq));
    CHECK(c.type == CommandType::Mechanism && c.value == 90);
}

static void testTelemetry() {
    char buf[MAX_PACKET + 1];
    PocketQubeTelemetry t;
    t.gnssLat = -38.12345; t.gnssLon = -122.12345; t.gnssAltM = 1234.5f;
    t.missionTimeS = 12345.678f; t.packetCount = 99999; t.commandCount = 99;
    for (int i = 0; i < 3; ++i) { t.rot[i] = -999.9f; t.accel[i] = -99.999f; t.mag[i] = -9999; }
    strcpy(t.gnssTime, "23:59:59");
    strcpy(t.cmdEcho, "MECCAM-90");
    size_t n = formatPocketQube(buf, sizeof(buf), t);
    printf("worst-case PocketQube line: %zu bytes (limit %zu)\n", n, MAX_PACKET);
    CHECK(n > 0);

    ContainerTelemetry ct;
    n = formatContainer(buf, sizeof(buf), ct);
    CHECK(n > 0 && buf[n - 1] == '\r');
}

static void testContainerFlight() {
    ContainerPersist p;
    ContainerFsm fsm(p);
    fsm.calibrate(101325.0f);
    fsm.arm();
    bool released = false;
    float releaseAlt = 0;
    uint32_t t = 0;
    // 10 s up to 700 m, then down at 10 m/s.
    for (; t <= 10000; t += 250) fsm.update(pressureAt(70.0f * t / 1000.0f), t);
    CHECK(p.state == ContainerState::Ascent);
    for (float alt = 700; alt > 0; alt -= 2.5f, t += 250) {
        ContainerOutputs o = fsm.update(pressureAt(alt), t);
        if (o.fireRelease) { released = true; releaseAlt = alt; }
    }
    for (int i = 0; i < 40; ++i, t += 250) fsm.update(pressureAt(0), t);
    CHECK(released);
    CHECK(releaseAlt <= 0.9f * 700 + 1 && releaseAlt > 0.85f * 700);
    CHECK(p.state == ContainerState::Landed);

    // After a "reset" with saved state, release must not fire again.
    ContainerFsm again(p);
    CHECK(!again.update(pressureAt(0), t).fireRelease);
}

static void testPocketQube() {
    PocketQubePersist p;
    PocketQubeFsm fsm(p);
    CHECK(!fsm.update(4999).firePanels);
    CHECK(fsm.update(5000).firePanels);
    CHECK(!fsm.update(6000).firePanels);
    CHECK(fsm.requestBoom());
    CHECK(!fsm.requestBoom());
}

int main() {
    testCommands();
    testTelemetry();
    testContainerFlight();
    testPocketQube();
    printf(failures ? "%d FAILED\n" : "all passed\n", failures);
    return failures ? 1 : 0;
}
