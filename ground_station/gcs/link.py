"""Serial link to the ground radio (ground_radio/ firmware)."""

import threading

import serial

from . import config


class Link:
    def __init__(self, port: str):
        self.ser = serial.Serial(port, config.BAUD, timeout=0.1)
        self.lock = threading.Lock()

    def read_line(self) -> str | None:
        raw = self.ser.readline()
        if not raw:
            return None
        line = raw.decode("ascii", errors="replace").strip()
        if not line or line.startswith("#"):  # '#' lines are radio status messages
            return None
        return line

    def send(self, vehicle: str, name: str, *args) -> str:
        """Send CMD,<team>,<C|P>,<name>[,args]. Returns the line sent."""
        fields = ["CMD", str(config.TEAM_ID), vehicle, name, *map(str, args)]
        line = ",".join(fields)
        with self.lock:
            self.ser.write((line + "\n").encode("ascii"))
        return line
