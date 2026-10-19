"""Writes the two judged CSV files (GS13) as packets arrive, and counts packets (GS11)."""

import os
import shutil

from . import config


class Recorder:
    def __init__(self, out_dir: str = "flight_data"):
        os.makedirs(out_dir, exist_ok=True)
        self.paths = {
            "C": os.path.join(out_dir, f"Flight_{config.TEAM_ID}C.csv"),
            "P": os.path.join(out_dir, f"Flight_{config.TEAM_ID}P.csv"),
        }
        headers = {"C": config.CONTAINER_HEADER, "P": config.POCKETQUBE_HEADER}
        self.files = {}
        for key, path in self.paths.items():
            new = not os.path.exists(path)
            # newline="" so we control line endings; the guide asks for a single CR.
            f = open(path, "a", newline="")
            if new:
                f.write(headers[key] + "\r")
            self.files[key] = f
        self.received = {"C": 0, "P": 0}

    def record(self, line: str) -> str | None:
        """Store a telemetry line. Returns 'C' or 'P', or None if it is not ours."""
        ident = line.split(",", 1)[0]
        if ident == f"{config.TEAM_ID}C":
            key = "C"
        elif ident == f"{config.TEAM_ID}P":
            key = "P"
        else:
            return None
        self.files[key].write(line + "\r")
        self.files[key].flush()
        self.received[key] += 1
        return key

    def export(self, usb_dir: str) -> list[str]:
        """Copy both files to the judges' USB stick in one step."""
        for f in self.files.values():
            f.flush()
        return [shutil.copy(p, usb_dir) for p in self.paths.values()]
