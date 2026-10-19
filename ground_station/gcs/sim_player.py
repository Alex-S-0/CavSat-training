"""Simulation mode (GS7, GS8, GS16): SIM ENABLE, SIM ACTIVATE, then one SIMP per second."""

import csv
import threading
import time

from . import config


def load_profile(path: str) -> list[int]:
    """Read pressures in pascals from the judges' profile file.

    TODO: confirm the file layout once the judges publish it; this takes the
    first numeric value on each row and skips headers and comments.
    """
    values = []
    with open(path, newline="") as f:
        for row in csv.reader(f):
            for cell in row:
                try:
                    values.append(round(float(cell)))
                    break
                except ValueError:
                    continue
    return values


class SimPlayer:
    def __init__(self, link, vehicle: str = "C"):
        self.link = link
        self.vehicle = vehicle
        self._stop = threading.Event()
        self.thread = None

    def start(self, profile: list[int]):
        self.link.send(self.vehicle, "SIM", "ENABLE")
        self.link.send(self.vehicle, "SIM", "ACTIVATE")
        self._stop.clear()
        self.thread = threading.Thread(target=self._run, args=(profile,), daemon=True)
        self.thread.start()

    def stop(self):
        self._stop.set()
        self.link.send(self.vehicle, "SIM", "DISABLE")

    def _run(self, profile):
        next_t = time.monotonic()
        for pa in profile:
            if self._stop.is_set():
                return
            self.link.send(self.vehicle, "SIMP", pa)
            next_t += config.SIM_PERIOD_S
            time.sleep(max(0.0, next_t - time.monotonic()))
