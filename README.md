# EIT Firmware

Firmware for an Electrical Impedance Tomography (EIT) system based on the ESP32-C3 microcontroller. This project controls signal generation, multiplexing, and data acquisition to perform impedance measurements.

## Hardware Architecture

The system is built around the **ESP32-C3** (RISC-V) and interfaces with the following key components:

*   **AD5930:** Waveform Generator (Signal Source).
*   **AD5270:** Digital Potentiometer (Gain Control for InAmps).
*   **ADG73:** Analog Multiplexer (Electrode Switching).
*   **AD7450:** Analog-to-Digital Converter (Data Acquisition).

## Project Structure

The codebase is organized into layers to separate hardware specifics from application logic:

```
EIT_Firmware/
├── Application_Layer/      # High-level logic
│   ├── calibration.c       # System calibration routines
│   └── measurement.c       # Impedance measurement sequences
├── Middle_Ware/            # Hardware Abstraction & Processing
│   ├── hardware.c          # HAL, DSP processing (FFT, Windowing)
│   ├── hardware-test.c     # Hardware verification suite
│   └── test_data_gen.c     # Synthetic data generation for DSP testing
├── Device_Drivers/         # Low-level Component Drivers
│   ├── AD5270_DigiPot.c    # AD5270 Driver
│   ├── AD5930_SigGen.c     # AD5930 Driver
│   ├── ADG73_MUX.c         # ADG73 Driver
│   └── AD7450_ADC.c        # AD7450 Driver
├── main/
│   └── main.c              # Entry point and task scheduling
└── CMakeLists.txt          # Build configuration
```

## Features

*   **Signal Processing:** Uses `esp-dsp` for FFT-based frequency and amplitude extraction.
    *   Includes Hanning windowing to reduce spectral leakage.
    *   Supports 500kHz sampling rate processing.
*   **Automated Testing:** Built-in hardware test suite (`hardware-test.c`) to verify:
    *   ADC communication.
    *   Signal Generator output.
    *   Multiplexer switching.
    *   DSP logic (using synthetic signals with random phase).
*   **Calibration:** Routines to calibrate input/output gain stages.

## Dependencies

This project relies on the **ESP-IDF** framework (v5.2+) and the following managed component:

*   `espressif/esp-dsp`: For FFT and math operations.

## Build and Flash

1.  **Set up ESP-IDF:** Ensure your environment is configured.
    ```bash
    . $HOME/esp/esp-idf/export.sh
    ```

2.  **Build the project:**
    ```bash
    idf.py build
    ```

3.  **Flash to device:**
    ```bash
    idf.py -p /dev/tty.usbserial-XXXX flash monitor
    ```

## Testing

The firmware includes a hardware test mode. To run the tests, ensure `run_hardware_test()` is called in `app_main` (currently enabled by default).

The DSP test generates synthetic sine waves (mixed frequencies) to verify the FFT implementation without requiring external hardware input.
