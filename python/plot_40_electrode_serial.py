#!/usr/bin/env python3
"""Live line-over-time plot of 40 EIT electrode channels — one subplot each."""

import argparse
import queue
import sys
import threading
import time
from typing import Optional

import matplotlib
matplotlib.use("MacOSX")          # fastest native backend on macOS
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np
import serial
from serial import SerialException
from serial.tools import list_ports

NUM_CHANNELS = 40
HISTORY_LEN = 100          # samples retained per channel (shorter = faster render)
SUBPLOT_ROWS = 8
SUBPLOT_COLS = 5           # 8 × 5 = 40 subplots
DEFAULT_BAUD = 115200
DEFAULT_TIMEOUT = 1.0
YLIM_UPDATE_EVERY = 10     # only rescale Y-axis every N rendered frames
PREFERRED_PORT_PREFIXES = (
    "/dev/cu.usbmodem",
    "/dev/tty.usbmodem",
    "/dev/cu.usbserial",
    "/dev/tty.usbserial",
)


def select_port(user_port: Optional[str]) -> str:
    if user_port:
        return user_port
    ports = [p.device for p in list_ports.comports()]
    for prefix in PREFERRED_PORT_PREFIXES:
        for port in ports:
            if port.startswith(prefix):
                return port
    if ports:
        return ports[0]
    raise RuntimeError("No serial ports found. Connect the device or pass --port explicitly.")


def parse_frame(raw: str) -> Optional[list[float]]:
    text = raw.strip()
    if not text:
        return None
    upper = text.upper()
    if upper == "TARE" or upper.startswith("TARE "):
        return None
    parts = text.replace(",", " ").split()
    if len(parts) < NUM_CHANNELS:
        return None
    try:
        return [float(p) for p in parts[:NUM_CHANNELS]]
    except ValueError:
        return None


def serial_reader(port: str, baud: int, timeout: float,
                  data_queue: queue.SimpleQueue, stop_event: threading.Event) -> None:
    """Background thread: open serial, push parsed frames onto data_queue."""
    ser: Optional[serial.Serial] = None
    while not stop_event.is_set():
        if ser is None or not ser.is_open:
            try:
                ser = serial.Serial(port, baud, timeout=timeout)
                time.sleep(2)
                ser.reset_input_buffer()
                data_queue.put(("status", f"Connected to {port} @ {baud}"))
                print(f"Connected to {port} @ {baud}")
            except SerialException as exc:
                data_queue.put(("status", f"Waiting for device: {exc}"))
                time.sleep(1)
                continue

        try:
            raw = ser.readline().decode("utf-8", errors="ignore")
        except SerialException as exc:
            data_queue.put(("status", f"Disconnected: {exc}"))
            print(f"Serial disconnected: {exc}")
            try:
                ser.close()
            except SerialException:
                pass
            ser = None
            continue

        frame = parse_frame(raw)
        if frame is not None:
            data_queue.put(("frame", frame))

    if ser is not None and ser.is_open:
        ser.close()
    print("Serial reader thread exited.")


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Live time-series line plot for 40 EIT channels"
    )
    parser.add_argument("--port", default=None, help="Serial port (auto-detected if omitted)")
    parser.add_argument("--baud", type=int, default=DEFAULT_BAUD)
    parser.add_argument("--timeout", type=float, default=DEFAULT_TIMEOUT)
    parser.add_argument(
        "--history", type=int, default=HISTORY_LEN,
        help="Number of samples to show per subplot",
    )
    args = parser.parse_args()

    try:
        port = select_port(args.port)
    except RuntimeError as exc:
        print(exc, file=sys.stderr)
        sys.exit(1)

    print(f"Using serial port: {port}")

    # numpy circular buffers — shape (NUM_CHANNELS, history)
    H = args.history
    buf = np.zeros((NUM_CHANNELS, H), dtype=np.float32)
    write_pos = 0   # next write index (circular)

    # Shared queue between serial thread and GUI
    data_queue: queue.SimpleQueue = queue.SimpleQueue()
    stop_event = threading.Event()
    reader_thread = threading.Thread(
        target=serial_reader,
        args=(port, args.baud, args.timeout, data_queue, stop_event),
        daemon=True,
    )

    # Build 8×5 grid of subplots
    plt.style.use("fast")
    fig, axes = plt.subplots(
        SUBPLOT_ROWS, SUBPLOT_COLS,
        figsize=(18, 20),
        sharex=False,
        sharey=False,
    )
    fig.suptitle("Live 40-Channel EIT — Value over Time", fontsize=14, y=1.002)
    axes_flat = axes.flatten()

    # Pre-build x index array (reused every frame, no allocation)
    x_idx = np.arange(H, dtype=np.float32)

    lines = []
    for ch_idx, ax in enumerate(axes_flat):
        (ln,) = ax.plot(x_idx, buf[ch_idx], linewidth=0.8, color="#4c78a8",
                        antialiased=False)
        ax.set_title(f"Ch {ch_idx + 1}", fontsize=7, pad=2)
        ax.set_xlim(0, H - 1)
        ax.set_ylim(0, 10)
        ax.tick_params(labelsize=6)
        ax.grid(True, linestyle=":", alpha=0.4)
        ax.set_xticks([])
        lines.append(ln)

    status_text = fig.text(0.01, 0.002, "Connecting...", ha="left", va="bottom", fontsize=8)

    ylim_cache = [(0.0, 10.0)] * NUM_CHANNELS
    frame_counter = [0]

    def update(_f):
        nonlocal write_pos

        # Drain the queue — consume all pending frames, keep only the latest
        latest_frame = None
        new_status = None
        try:
            while True:
                kind, payload = data_queue.get_nowait()
                if kind == "frame":
                    latest_frame = payload
                else:
                    new_status = payload
        except queue.Empty:
            pass

        if new_status is not None:
            status_text.set_text(new_status)

        if latest_frame is None:
            return lines

        # Write new sample into circular buffer column
        buf[:, write_pos] = latest_frame
        write_pos = (write_pos + 1) % H

        # Reconstruct time-ordered view without copying (roll via index)
        ordered = np.concatenate(
            (buf[:, write_pos:], buf[:, :write_pos]), axis=1
        )  # shape (40, H)

        for ch_idx in range(NUM_CHANNELS):
            lines[ch_idx].set_ydata(ordered[ch_idx])

        # Rescale Y only every YLIM_UPDATE_EVERY frames to avoid axis overhead
        frame_counter[0] += 1
        if frame_counter[0] % YLIM_UPDATE_EVERY == 0:
            for ch_idx in range(NUM_CHANNELS):
                lo = float(ordered[ch_idx].min())
                hi = float(ordered[ch_idx].max())
                pad = max(1.0, (hi - lo) * 0.1)
                new_lim = (lo - pad, hi + pad)
                if new_lim != ylim_cache[ch_idx]:
                    axes_flat[ch_idx].set_ylim(new_lim)
                    ylim_cache[ch_idx] = new_lim

        status_text.set_text(
            f"Frame | min={min(latest_frame):.0f}  max={max(latest_frame):.0f}"
            f"  mean={sum(latest_frame)/len(latest_frame):.1f}"
        )
        return lines

    reader_thread.start()

    ani = FuncAnimation(fig, update, interval=30, blit=False, cache_frame_data=False)
    _ = ani

    try:
        plt.tight_layout()
        plt.show()
    finally:
        stop_event.set()
        reader_thread.join(timeout=3)
        print("Done.")


if __name__ == "__main__":
    main()


if __name__ == "__main__":
    main()
