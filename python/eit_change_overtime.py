#!/usr/bin/env python3
"""
Visualize 40‑channel drift over time.

Usage:
  python plot_40ch_drift.py frames.txt
"""

import sys
import numpy as np
import matplotlib.pyplot as plt

NUM_CH = 40

def load_frames(path: str) -> np.ndarray:
    frames = []
    with open(path, "r") as f:
        for line_no, line in enumerate(f, start=1):
            line = line.strip()
            if not line:
                continue
            parts = line.split()
            if len(parts) != NUM_CH:
                raise ValueError(
                    f"Line {line_no}: expected {NUM_CH} values, got {len(parts)}"
                )
            frames.append([float(x) for x in parts])
    if not frames:
        raise ValueError("No frames loaded.")
    return np.asarray(frames)  # (num_frames, NUM_CH)

def main():
    if len(sys.argv) != 2:
        print("Usage: python plot_40ch_drift.py frames.txt")
        sys.exit(1)

    frames = load_frames(sys.argv[1])
    num_frames, num_ch = frames.shape

    t = np.arange(num_frames)

    # 1) Heatmap: drift vs first frame
    ref = frames[0]
    deltas = frames - ref  # (frames, channels)

    plt.figure(figsize=(10, 6))
    plt.imshow(deltas.T, aspect="auto", origin="lower",
               interpolation="nearest", cmap="coolwarm")
    plt.colorbar(label="Δ value vs frame 0")
    plt.xlabel("Frame index")
    plt.ylabel("Channel")
    plt.yticks(np.arange(num_ch))
    plt.title("Per‑channel drift vs first frame")
    
    # 2) Overlay of all channels (raw)
    plt.figure(figsize=(10, 5))
    for ch in range(num_ch):
        plt.plot(t, frames[:, ch], alpha=0.4)
    plt.xlabel("Frame index")
    plt.ylabel("Value")
    plt.title("All channels over time")

    # 3) Max |drift| per channel bar plot
    abs_drift = np.max(np.abs(deltas), axis=0)
    plt.figure(figsize=(10, 4))
    plt.bar(np.arange(num_ch), abs_drift)
    plt.xlabel("Channel")
    plt.ylabel("Max |Δ vs frame 0|")
    plt.title("Max absolute drift per channel")

    plt.tight_layout()
    plt.show()

    # 4) One window per channel (raw time series), shown sequentially.
    # Close a window to advance to the next channel.
    for ch in range(num_ch):
        fig, ax = plt.subplots(figsize=(8, 4))
        ax.plot(t, frames[:, ch])
        ax.set_xlabel("Frame index")
        ax.set_ylabel("Value")
        ax.set_title(f"Channel {ch} over time")
        ax.grid(True, alpha=0.3)
        plt.show()   # blocks until this window is closed
        plt.close(fig)

if __name__ == "__main__":
    main()