"""Minimal console ground station: prints and records telemetry, sends typed commands.

Run:  python -m gcs --port /dev/tty.usbmodemXXXX
Type commands as  C CAL   P MEC BOOM ON   C SIMFILE profile.csv   EXPORT /Volumes/USB

TODO (Communication): replace the console with the real display (plots, GS1-GS5,
sunlight theme GS12) once the UI stack is chosen.
"""

import argparse
import threading

from .link import Link
from .recorder import Recorder
from .sim_player import SimPlayer, load_profile


def reader(link: Link, rec: Recorder):
    while True:
        line = link.read_line()
        if line is None:
            continue
        key = rec.record(line)
        if key:
            print(f"[{key} rx={rec.received[key]}] {line}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", required=True)
    args = ap.parse_args()

    link = Link(args.port)
    rec = Recorder()
    threading.Thread(target=reader, args=(link, rec), daemon=True).start()
    sim = None

    while True:
        parts = input("> ").split()
        if not parts:
            continue
        if parts[0].upper() == "EXPORT" and len(parts) == 2:
            print("copied:", rec.export(parts[1]))
        elif len(parts) >= 3 and parts[1].upper() == "SIMFILE":
            sim = SimPlayer(link, parts[0].upper())
            sim.start(load_profile(parts[2]))
        elif len(parts) >= 2 and parts[1].upper() == "SIMSTOP" and sim:
            sim.stop()
        elif parts[0].upper() in ("C", "P") and len(parts) >= 2:
            print("sent:", link.send(parts[0].upper(), *[p.upper() for p in parts[1:]]))
        else:
            print("usage: C|P <COMMAND> [ARGS]  |  C|P SIMFILE <file>  |  C|P SIMSTOP  |  EXPORT <dir>")


if __name__ == "__main__":
    main()
