from __future__ import annotations

import argparse
from datetime import datetime
from pathlib import Path
import sys

try:
    import serial
except ImportError:
    print("Missing dependency: pyserial. Install it with `pip install pyserial`.", file=sys.stderr)
    raise SystemExit(1)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Read ESP32 serial output and save TARE/MEAS lines to a timestamped txt file."
    )
    parser.add_argument("port", help="Serial port, for example /dev/tty.usbmodem2101")
    parser.add_argument("--baud", type=int, default=115200, help="Serial baud rate")
    parser.add_argument("--output-dir", default="logs", help="Directory for created log files")
    return parser.parse_args()


def main() -> None:
    args = parse_args()

    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    created_at = datetime.now()
    log_path = output_dir / f"eit_log_{created_at.strftime('%Y-%m-%d_%H-%M-%S')}.txt"

    with serial.Serial(args.port, args.baud, timeout=1) as ser, log_path.open("w", encoding="utf-8") as log_file:
        log_file.write(f"created_at: {created_at.strftime('%Y-%m-%d %H:%M:%S')}\n")
        log_file.write(f"port: {args.port}\n")
        log_file.write(f"baud: {args.baud}\n\n")
        log_file.flush()

        print(f"Logging serial data to {log_path}")

        while True:
            raw_line = ser.readline()
            if not raw_line:
                continue

            line = raw_line.decode("utf-8", errors="replace").strip()
            if not line:
                continue

            if line.startswith("TARE:") or line.startswith("MEAS:"):
                print(line)
                log_file.write(line + "\n")
                log_file.flush()


if __name__ == "__main__":
    main()
