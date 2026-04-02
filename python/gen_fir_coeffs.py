#!/usr/bin/env python3
"""Generate FIR bandpass filter coefficients for EIT measurement."""
import numpy as np
from scipy.signal import firwin, freqz

fs = 500000    # 500 kHz ADC sample rate
num_taps = 21  # 21-tap FIR

# Bandpass: 30 kHz to 70 kHz (centered on 50 kHz signal)
coeffs = firwin(num_taps, [30000, 70000], pass_zero=False, fs=fs, window='hamming')

print(f"#define FIR_NUM_TAPS {num_taps}")
print("static const float fir_coeffs[FIR_NUM_TAPS] = {")
for i, c in enumerate(coeffs):
    comma = "," if i < len(coeffs) - 1 else ""
    print(f"    {c: .10f}f{comma}")
print("};")

w, h = freqz(coeffs, worN=1024, fs=fs)
idx_50k = np.argmin(np.abs(w - 50000))
idx_dc = 0
idx_100k = np.argmin(np.abs(w - 100000))
print(f"// Gain at DC:     {20*np.log10(abs(h[idx_dc])+1e-15):.1f} dB")
print(f"// Gain at 50kHz:  {20*np.log10(abs(h[idx_50k])+1e-15):.1f} dB")
print(f"// Gain at 100kHz: {20*np.log10(abs(h[idx_100k])+1e-15):.1f} dB")
print(f"// Usable output samples: 64 - {num_taps-1} = {64 - (num_taps-1)}")
