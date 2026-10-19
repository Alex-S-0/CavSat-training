# Software layout

| Folder | Owner | Language | What it is |
|---|---|---|---|
| `lib/cavsat_common/` | Software | C++ | Shared code: config, command parser, telemetry format, altitude, state machines, ESP-NOW radio, saved state, SD logger |
| `container/` | Software | C++ (PlatformIO) | Container flight software |
| `pocketqube/` | Software | C++ (PlatformIO) | PocketQube flight software, camera stabilization |
| `ground_radio/` | Communication | C++ (PlatformIO) | ESP32 on USB that bridges ESP-NOW to the laptop |
| `ground_station/` | Communication | Python | Ground station: link, CSV recording, simulation player |
| `docs/ICD.md` | both | | Command and telemetry formats |
| `tools/` | both | Python | Helpers such as the CSV format checker |

## Build and flash

Install PlatformIO (VS Code extension or `pip install platformio`), then:

```
cd container && pio run -t upload && pio device monitor
```

Same for `pocketqube/` and `ground_radio/`. Board is a placeholder
(`esp32-s3-devkitc-1`); change it in each `platformio.ini` once the boards are chosen.

## Host tests (no hardware)

The flight logic in `lib/cavsat_common` runs on a laptop:

```
cd lib/cavsat_common/test
g++ -std=c++17 -I../src test_host.cpp ../src/*.cpp -o test_host && ./test_host
```

## Before first flight, fill in

- `TEAM_ID`, `ESPNOW_CHANNEL` and the three MAC addresses in `lib/cavsat_common/src/config.h` and `ground_station/gcs/config.py`.
- Pins in each `src/pins.h` once Electronics fixes the wiring.
- Sensor drivers in each `src/sensors.cpp` (marked TODO).
- Thresholds in `config.h` after drop and vacuum tests.
