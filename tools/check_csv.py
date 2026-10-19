#!/usr/bin/env python3
"""Check a flight CSV against the competition format before handing it to the judges.

Usage: python3 tools/check_csv.py ground_station/flight_data/Flight_1000C.csv
"""

import sys

sys.path.insert(0, __file__.rsplit("/tools/", 1)[0] + "/ground_station")
from gcs import config  # noqa: E402


def check(path: str) -> list[str]:
    problems = []
    data = open(path, "rb").read()
    if b"\n" in data:
        problems.append("contains LF; lines must end with a single CR")
    lines = [l for l in data.decode("ascii", errors="replace").split("\r") if l]
    if not lines:
        return ["file is empty"]
    vehicle = "C" if path.removesuffix(".csv").endswith("C") else "P"
    header = config.CONTAINER_HEADER if vehicle == "C" else config.POCKETQUBE_HEADER
    if lines[0] != header:
        problems.append("header row does not match the expected columns")
    ncols = header.count(",") + 1
    last_count = None
    for i, line in enumerate(lines[1:], start=2):
        fields = line.split(",")
        if len(fields) != ncols:
            problems.append(f"line {i}: {len(fields)} fields, expected {ncols}")
            continue
        if fields[0] != f"{config.TEAM_ID}{vehicle}":
            problems.append(f"line {i}: ID {fields[0]!r}")
        count_idx = 2 if vehicle == "C" else 3
        try:
            count = int(fields[count_idx])
        except ValueError:
            problems.append(f"line {i}: PACKET_COUNT not an integer")
            continue
        if last_count is not None and count <= last_count:
            problems.append(f"line {i}: PACKET_COUNT went from {last_count} to {count}")
        last_count = count
    return problems


if __name__ == "__main__":
    bad = False
    for p in sys.argv[1:]:
        found = check(p)
        print(f"{p}: {'OK' if not found else str(len(found)) + ' problem(s)'}")
        for f in found[:20]:
            print("  ", f)
        bad |= bool(found)
    sys.exit(1 if bad else 0)
