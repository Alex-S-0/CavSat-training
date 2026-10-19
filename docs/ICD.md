# Interface control: commands and telemetry

Shared by Software (container and PocketQube flight code) and Communication
(ground radio and ground station). The code version is
`lib/cavsat_common/src/protocol.h` and `ground_station/gcs/config.py`; change all
three together.

## Radio

- ESP-NOW on every link, unicast to a known MAC, never broadcast.
- Channel and MAC addresses live in `lib/cavsat_common/src/config.h` (TODO: fill in).
- One packet is one ASCII line, at most 250 bytes. The worst-case PocketQube
  line is 176 bytes (checked by the host test).

## Uplink commands

Syntax: `CMD,<TEAM_ID>,<C|P>,<NAME>[,<ARG1>[,<ARG2>]]`, one per line ending `\n`.
The vehicle ignores lines with another team ID or addressed to the other vehicle.

| Command | Vehicle | Meaning | Echo |
|---|---|---|---|
| `ST,<hh:mm:ss>` / `ST,GPS` | C, P | Set mission time | `STGPS` |
| `SIM,ENABLE` / `SIM,ACTIVATE` / `SIM,DISABLE` | C | Simulation mode (needs ENABLE then ACTIVATE) | `SIMENABLE` |
| `SIMP,<pascals>` | C | Simulated pressure, sent at 1 Hz | `SIMP101325` |
| `CAL` | C | Zero altitude at the pad and reset counters | `CAL` |
| `ARM` | C | Allow launch detection | `ARM` |
| `MEC,REL,ON\|OFF` | C | Drive the PocketQube release (test only) | `MECRELON` |
| `MEC,CAM,ON\|OFF` | C | Container camera | `MECCAMON` |
| `MEC,BOOM,ON` | P | Deploy the boom (once) | `MECBOOMON` |
| `MEC,PANEL,ON` | P | Fire the solar panel release | `MECPANELON` |
| `MEC,CAM,<degrees>` | P | Rotate the stabilized camera from north | `MECCAM90` |
| `MEC,REC,ON` | P | Start video recording | `MECRECON` |
| `RESET` | C, P | Clear saved state (testing only, never in flight) | `RESET` |

CMD_ECHO carries the last accepted command: name and arguments with the commas removed.

TODO: confirm the exact command names against the final guide revision
(the 2027 guide is inconsistent on SIM naming and the video command).

## Downlink telemetry

ASCII CSV, fields in this order, each line ending with a single `\r`.

Container (4 Hz from power-on, 1 Hz from 5 s after release, stops after landing):

```
ID,MISSION_TIME,PACKET_COUNT,COMMAND_COUNT,MODE,ALTITUDE,PRESSURE,TEMPERATURE,BATT_V,BATT_I,MECH_STATE,STATE,CMD_ECHO
```

PocketQube (4 Hz from release):

```
ID,MODE,MISSION_TIME,PACKET_COUNT,COMMAND_COUNT,ALTITUDE,TEMPERATURE,PRESSURE,VOLTAGE,CURRENT,GNSS_TIME,GNSS_ALTITUDE,GNSS_LATITUDE,GNSS_LONGITUDE,GNSS_SATS,ROT_RATE_X,ROT_RATE_Y,ROT_RATE_Z,ACCEL_X,ACCEL_Y,ACCEL_Z,MAG_X,MAG_Y,MAG_Z,SOLAR_1,SOLAR_2,MECH_STATE,CMD_ECHO,STAB_ERROR,STAB_COMMAND,SCIENCE_EXP
```

- `ID` is the team number plus `C` or `P`, e.g. `1000C`.
- `MODE` is `F` (flight) or `S` (simulation).
- Container `STATE`: `LAUNCH_PAD`, `ASCENT`, `APOGEE`, `PQ_RELEASE`, `LANDED`
  (TODO: confirm `LANDED` is accepted; the guide lists only the first four).
- `IMAGE_STABILIZATION` is team-defined; we send heading error and commanded angle in degrees.

### MECH_STATE bits

| Bit | Container | PocketQube |
|---|---|---|
| 0 | release activated | panel 1 deployed |
| 1 | PocketQube released | panel 2 deployed |
| 2 | | boom deployed |
| 3 | | camera recording |

## Ground station files

`Flight_<TEAM_ID>C.csv` and `Flight_<TEAM_ID>P.csv`, header row first, each line
ending in `\r`, copied to the judges' USB stick at the end of the flight.
