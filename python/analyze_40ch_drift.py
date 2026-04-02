#!/usr/bin/env python3
"""
Analyze drift over time for 40-channel frames.

Usage:
  python analyze_40ch_drift.py path/to/frames.txt

Input format:
  - Each line = one frame
  - 40 space-separated values per line (ints or floats)
"""

import sys
import numpy as np

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
            vals = [float(x) for x in parts]
            frames.append(vals)
    if not frames:
        raise ValueError("No frames loaded.")
    return np.asarray(frames)  # shape: (num_frames, NUM_CH)


def main() -> None:
    if len(sys.argv) != 2:
        print("Usage: python analyze_40ch_drift.py path/to/frames.txt")
        sys.exit(1)

    path = sys.argv[1]
    frames = load_frames(path)
    num_frames, num_ch = frames.shape
    if num_ch != NUM_CH:
        print(f"Warning: expected {NUM_CH} channels, found {num_ch}")

    print(f"Loaded {num_frames} frames, {num_ch} channels per frame.\n")

    # Basic stats per channel over time
    ch_mean = frames.mean(axis=0)
    ch_std = frames.std(axis=0)

    # Drift relative to first frame
    ref = frames[0, :]  # frame 0
    deltas = frames - ref  # shape (num_frames, NUM_CH)
    drift_min = deltas.min(axis=0)
    drift_max = deltas.max(axis=0)
    drift_mean = deltas.mean(axis=0)

    print("Per-channel stats (over time):")
    print("ch  mean       std        drift_mean  drift_min   drift_max")
    print("--  ---------  ---------  ----------  ---------  ---------")
    for ch in range(num_ch):
        print(
            f"{ch:2d}  "
            f"{ch_mean[ch]:9.3f}  {ch_std[ch]:9.3f}  "
            f"{drift_mean[ch]:10.3f}  {drift_min[ch]:9.3f}  {drift_max[ch]:9.3f}"
        )

    # Identify channels with largest absolute drift vs first frame
    abs_drift_max = np.maximum(np.abs(drift_min), np.abs(drift_max))
    order = np.argsort(-abs_drift_max)  # descending
    top_k = min(5, num_ch)

    print("\nTop channels by max |drift| relative to first frame:")
    print("rank  ch   max_|drift|")
    print("----  --   ----------")
    for rank in range(top_k):
        ch = int(order[rank])
        print(f"{rank+1:4d}  {ch:2d}   {abs_drift_max[ch]:10.3f}")

    print("\nDone.")


if __name__ == "__main__":
    main()

