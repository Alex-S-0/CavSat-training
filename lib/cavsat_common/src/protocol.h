// Command and telemetry definitions shared by the vehicles and the ground radio.
// docs/ICD.md is the human-readable version; keep the two in step.
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace cavsat {

// ---------------------------------------------------------------- commands
// Uplink syntax:  CMD,<TEAM_ID>,<VEHICLE>,<NAME>[,<ARG1>[,<ARG2>]]
//   VEHICLE is C (container) or P (PocketQube).
//   e.g. CMD,1000,C,SIM,ENABLE   CMD,1000,C,SIMP,101325   CMD,1000,P,MEC,CAM,90
// Lines end with '\n'. The echo sent back in CMD_ECHO is NAME and ARGs joined
// without commas, e.g. SIMENABLE, SIMP101325, MECCAM90.

enum class Vehicle : uint8_t { Container, PocketQube };

enum class CommandType : uint8_t {
    Invalid,
    SetTime,    // ST,<hh:mm:ss> or ST,GPS            (F3)
    Sim,        // SIM,ENABLE|ACTIVATE|DISABLE         (F4-F6)
    SimPressure,// SIMP,<pascals>                      (F5)
    Calibrate,  // CAL                                 (container only)
    Arm,        // ARM                                 (container only)
    Mechanism,  // MEC,<device>,<ON|OFF|degrees>       (F7, CTR17, PQ19)
    Reset,      // RESET  (testing only)
};

enum class SimArg : uint8_t { None, Enable, Activate, Disable };

constexpr size_t MAX_FIELD = 16;

struct Command {
    CommandType type = CommandType::Invalid;
    Vehicle vehicle = Vehicle::Container;
    SimArg sim = SimArg::None;
    char arg1[MAX_FIELD] = {0};  // device name, time string, ...
    char arg2[MAX_FIELD] = {0};  // ON, OFF or a number
    int32_t value = 0;           // SIMP pressure, MEC degrees
    char echo[32] = {0};         // text for CMD_ECHO
};

// Parse one uplink line. Returns false for a malformed line, another team's
// ID, or (when 'self' is given) a command addressed to the other vehicle.
bool parseCommand(const char* line, Command& out, const Vehicle* self = nullptr);

// Build an uplink line, used by the ground station and by tests.
size_t formatCommand(char* buf, size_t len, Vehicle v, const char* name,
                     const char* arg1 = nullptr, const char* arg2 = nullptr);

// --------------------------------------------------------------- telemetry
// Downlink is the ASCII CSV line itself, ending in '\r' (guide: single CR).

enum class ContainerState : uint8_t { LaunchPad, Ascent, Apogee, PqRelease, Landed };
const char* toString(ContainerState s);

// Container MECH_STATE bits.
constexpr uint8_t CMECH_RELEASE_ACTIVATED = 1 << 0;
constexpr uint8_t CMECH_PQ_RELEASED = 1 << 1;

// PocketQube MECH_STATE bits.
constexpr uint8_t PMECH_PANEL1 = 1 << 0;
constexpr uint8_t PMECH_PANEL2 = 1 << 1;
constexpr uint8_t PMECH_BOOM = 1 << 2;
constexpr uint8_t PMECH_CAMERA_REC = 1 << 3;

struct ContainerTelemetry {
    float missionTimeS = 0;   // 1 ms resolution
    uint32_t packetCount = 0;
    uint32_t commandCount = 0;
    bool simMode = false;
    float altitudeM = 0;      // 0.1 m
    int32_t pressurePa = 0;   // 1 Pa
    float temperatureC = 0;   // 0.1 C
    float batteryV = 0;       // 0.01 V (CTR21)
    int32_t batteryMa = 0;    // 1 mA
    uint8_t mechState = 0;
    ContainerState state = ContainerState::LaunchPad;
    char cmdEcho[32] = {0};
};

struct PocketQubeTelemetry {
    float missionTimeS = 0;
    uint32_t packetCount = 0;
    uint32_t commandCount = 0;
    bool simMode = false;
    float altitudeM = 0;
    float temperatureC = 0;
    int32_t pressurePa = 0;
    float batteryV = 0;       // 0.1 V
    int32_t batteryMa = 0;    // 1 mA
    char gnssTime[12] = {0};  // hh:mm:ss
    float gnssAltM = 0;
    double gnssLat = 0;
    double gnssLon = 0;
    uint8_t gnssSats = 0;
    float rot[3] = {0, 0, 0};   // deg/s, 0.1
    float accel[3] = {0, 0, 0}; // m/s^2, 0.001
    int32_t mag[3] = {0, 0, 0}; // milligauss, 1
    float solar1V = 0;          // 0.01 V
    float solar2V = 0;
    uint8_t mechState = 0;
    char cmdEcho[32] = {0};
    float stabErrorDeg = 0;     // IMAGE_STABILIZATION fields (team-defined)
    float stabCommandDeg = 0;
    char science[32] = {0};     // SCIENCE_EXP, empty unless the bonus is flown
};

// Header rows for the ground station CSV files (GS13).
extern const char* CONTAINER_CSV_HEADER;
extern const char* POCKETQUBE_CSV_HEADER;

// Format one CSV line in the guide's column order. Returns its length, or 0
// if it did not fit in buf.
size_t formatContainer(char* buf, size_t len, const ContainerTelemetry& t);
size_t formatPocketQube(char* buf, size_t len, const PocketQubeTelemetry& t);

} // namespace cavsat
