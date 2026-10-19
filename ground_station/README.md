# Ground station (Communication)

Python 3.10+. Talks to the `ground_radio/` ESP32 over USB serial.

```
python -m venv .venv && source .venv/bin/activate
pip install -r requirements.txt
python -m gcs --port /dev/tty.usbmodemXXXX
```

What is here: serial link, command sender, live CSV recording with the judged
file names (`Flight_XXXXC.csv`, `Flight_XXXXP.csv`), received-packet counts,
the simulation-mode player, and one-step export to USB.

Still to build: the real display (required plots, orientation, GPS, sunlight
theme), command buttons for every mechanism, set-time from GPS, spoken callouts.
