#include "protocol.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace cavsat {

namespace {

// Split a comma-separated line in place. Returns the number of fields.
size_t split(char* s, char* fields[], size_t maxFields) {
    size_t n = 0;
    char* p = s;
    while (n < maxFields) {
        fields[n++] = p;
        char* comma = strchr(p, ',');
        if (!comma) break;
        *comma = '\0';
        p = comma + 1;
    }
    return n;
}

void copyField(char* dst, const char* src) {
    strncpy(dst, src, MAX_FIELD - 1);
    dst[MAX_FIELD - 1] = '\0';
}

} // namespace

bool parseCommand(const char* line, Command& out, const Vehicle* self) {
    char buf[96];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    // Strip line endings.
    for (char* p = buf; *p; ++p) {
        if (*p == '\r' || *p == '\n') { *p = '\0'; break; }
    }

    char* f[7];
    size_t n = split(buf, f, 7);
    if (n < 4 || strcmp(f[0], "CMD") != 0) return false;
    if (atoi(f[1]) != TEAM_ID) return false;

    out = Command{};
    if (strcmp(f[2], "C") == 0) out.vehicle = Vehicle::Container;
    else if (strcmp(f[2], "P") == 0) out.vehicle = Vehicle::PocketQube;
    else return false;
    if (self && out.vehicle != *self) return false;

    const char* name = f[3];
    const char* a1 = n > 4 ? f[4] : "";
    const char* a2 = n > 5 ? f[5] : "";

    if (strcmp(name, "ST") == 0 && *a1) {
        out.type = CommandType::SetTime;
        copyField(out.arg1, a1);
    } else if (strcmp(name, "SIM") == 0) {
        out.type = CommandType::Sim;
        if (strcmp(a1, "ENABLE") == 0) out.sim = SimArg::Enable;
        else if (strcmp(a1, "ACTIVATE") == 0) out.sim = SimArg::Activate;
        else if (strcmp(a1, "DISABLE") == 0) out.sim = SimArg::Disable;
        else return false;
    } else if (strcmp(name, "SIMP") == 0 && *a1) {
        out.type = CommandType::SimPressure;
        out.value = atol(a1);
    } else if (strcmp(name, "CAL") == 0) {
        out.type = CommandType::Calibrate;
    } else if (strcmp(name, "ARM") == 0) {
        out.type = CommandType::Arm;
    } else if (strcmp(name, "MEC") == 0 && *a1 && *a2) {
        out.type = CommandType::Mechanism;
        copyField(out.arg1, a1);
        copyField(out.arg2, a2);
        out.value = atol(a2);
    } else if (strcmp(name, "RESET") == 0) {
        out.type = CommandType::Reset;
    } else {
        return false;
    }

    // CMD_ECHO: name and args with no commas (time colons are kept).
    snprintf(out.echo, sizeof(out.echo), "%s%s%s", name, a1, a2);
    return true;
}

size_t formatCommand(char* buf, size_t len, Vehicle v, const char* name,
                     const char* arg1, const char* arg2) {
    int n = snprintf(buf, len, "CMD,%u,%c,%s%s%s%s%s\n", TEAM_ID,
                     v == Vehicle::Container ? 'C' : 'P', name,
                     arg1 ? "," : "", arg1 ? arg1 : "",
                     arg2 ? "," : "", arg2 ? arg2 : "");
    return (n > 0 && (size_t)n < len) ? (size_t)n : 0;
}

const char* toString(ContainerState s) {
    switch (s) {
        case ContainerState::LaunchPad: return "LAUNCH_PAD";
        case ContainerState::Ascent: return "ASCENT";
        case ContainerState::Apogee: return "APOGEE";
        case ContainerState::PqRelease: return "PQ_RELEASE";
        case ContainerState::Landed: return "LANDED";  // TODO: confirm judges accept an extra state
    }
    return "UNKNOWN";
}

const char* CONTAINER_CSV_HEADER =
    "ID,MISSION_TIME,PACKET_COUNT,COMMAND_COUNT,MODE,ALTITUDE,PRESSURE,"
    "TEMPERATURE,BATT_V,BATT_I,MECH_STATE,STATE,CMD_ECHO";

const char* POCKETQUBE_CSV_HEADER =
    "ID,MODE,MISSION_TIME,PACKET_COUNT,COMMAND_COUNT,ALTITUDE,TEMPERATURE,"
    "PRESSURE,VOLTAGE,CURRENT,GNSS_TIME,GNSS_ALTITUDE,GNSS_LATITUDE,"
    "GNSS_LONGITUDE,GNSS_SATS,ROT_RATE_X,ROT_RATE_Y,ROT_RATE_Z,ACCEL_X,"
    "ACCEL_Y,ACCEL_Z,MAG_X,MAG_Y,MAG_Z,SOLAR_1,SOLAR_2,MECH_STATE,CMD_ECHO,"
    "STAB_ERROR,STAB_COMMAND,SCIENCE_EXP";

size_t formatContainer(char* buf, size_t len, const ContainerTelemetry& t) {
    int n = snprintf(buf, len,
        "%uC,%.3f,%lu,%lu,%c,%.1f,%ld,%.1f,%.2f,%ld,%X,%s,%s\r",
        TEAM_ID, t.missionTimeS, (unsigned long)t.packetCount,
        (unsigned long)t.commandCount, t.simMode ? 'S' : 'F', t.altitudeM,
        (long)t.pressurePa, t.temperatureC, t.batteryV, (long)t.batteryMa,
        t.mechState, toString(t.state), t.cmdEcho);
    return (n > 0 && (size_t)n < len) ? (size_t)n : 0;
}

size_t formatPocketQube(char* buf, size_t len, const PocketQubeTelemetry& t) {
    int n = snprintf(buf, len,
        "%uP,%c,%.3f,%lu,%lu,%.1f,%.1f,%ld,%.1f,%ld,%s,%.1f,%.5f,%.5f,%u,"
        "%.1f,%.1f,%.1f,%.3f,%.3f,%.3f,%ld,%ld,%ld,%.2f,%.2f,%X,%s,%.1f,%.1f,%s\r",
        TEAM_ID, t.simMode ? 'S' : 'F', t.missionTimeS,
        (unsigned long)t.packetCount, (unsigned long)t.commandCount,
        t.altitudeM, t.temperatureC, (long)t.pressurePa, t.batteryV,
        (long)t.batteryMa, t.gnssTime, t.gnssAltM, t.gnssLat, t.gnssLon,
        t.gnssSats, t.rot[0], t.rot[1], t.rot[2], t.accel[0], t.accel[1],
        t.accel[2], (long)t.mag[0], (long)t.mag[1], (long)t.mag[2],
        t.solar1V, t.solar2V, t.mechState, t.cmdEcho, t.stabErrorDeg,
        t.stabCommandDeg, t.science);
    return (n > 0 && (size_t)n < len) ? (size_t)n : 0;
}

} // namespace cavsat
