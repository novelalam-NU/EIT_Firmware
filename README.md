# EITWatch Firmware

Firmware for an **electrical impedance tomography (EIT)** front end on **Espressif ESP32** (ESP-IDF). It drives an **AD5930** waveform source, **ADG73** multiplexers, **AD5270** instrumentation-amp gains, and reads samples through an **AD7450** ADC over SPI. A FreeRTOS **measurement** task steps a fixed electrode-pair map, smooths per-channel amplitude (EWMA), and a **UDP** task streams the data over Wi-Fi.

## MCU and pins

- **Build target** is set in `sdkconfig` (e.g. `CONFIG_IDF_TARGET="esp32s3"`). Use `idf.py set-target <chip>` to match your module.
- **Pin assignments** live in `Middle_Ware/hardware.h` (currently aligned with **Seeed XIAO ESP32-C3**-style SPI and chip selects). Adjust if your PCB differs.
- **`ESP32-with-mappings.png`** (repo root) is a visual pin/reference diagram for the XIAO ESP32-C3 mapping used during development.

## Hardware blocks

| Part    | Role |
|--------|------|
| **AD5930** | Excitation / signal generator (SPI) |
| **AD5270** | Dual digital potentiometer (SPI) — source and sense in-amp gain |
| **ADG73**  | Analog multiplexers — electrode routing |
| **AD7450** | SPI ADC — sense channel acquisition |

On-chip ESP32 ADC is **not** used for EIT acquisition; sampling goes through the **AD7450**.

## Repository layout

```
EITWatch_Firmware/
├── Application_Layer/
│   ├── calibration.c / calibration.h   # Electrode pair map, EWMA buffer, calibrate()
│   ├── measurement.c / measurement.h # Scan loop, amplitude metrics, notifies UDP task
│   ├── tasks.c / tasks.h               # start_measurement_task(), start_udp_task()
│   └── wireless.c / wireless.h       # Wi-Fi STA, UDP socket, broadcast send
├── Middle_Ware/
│   ├── hardware.c / hardware.h         # SPI HAL, mux, ADC read, AD5930 control
│   └── CMakeLists.txt
├── Device_Drivers/                     # AD5930, AD7450, AD5270, ADG73 drivers
├── main/
│   ├── main.c                          # app_main: HW init → Wi-Fi → UDP socket → tasks
│   └── CMakeLists.txt
├── python/                             # Host-side helpers (plots, streams, analysis)
├── CMakeLists.txt                      # project(EITWatch_Firmware)
└── sdkconfig                           # IDF configuration (target, Wi-Fi, etc.)
```

## Runtime behavior

1. **`app_main`** initializes SPI, ADC, mux, pots, AD5930 frequency, then Wi-Fi (station), optional wait for DHCP, then creates a **UDP** socket (broadcast mode; see `wireless.c` / `wireless.h`).
2. **`start_measurement_task()`** runs the measurement loop on one core; **`start_udp_task()`** waits on task notifications and sends **`ewma_amp`** payloads over UDP.
3. **Electrode configuration** is a compile-time **`pair_calibration_map`** (see `calibration.c`) with **`NUM_ELECTRODE_PAIRS × NUM_SENSE_PAIRS`** channels (EWMA array size matches).

Wi-Fi SSID/password and UDP port are defined in **`Application_Layer/wireless.h`** (change before deployment).

## Python utilities

Scripts under **`python/`** are for host PCs (serial plots, logging, drift analysis, etc.). They are not part of the ESP-IDF firmware build. Use a local Python 3 environment with the dependencies each script needs (e.g. NumPy, PySerial, Matplotlib).

## Requirements

- **ESP-IDF** v5.x (project has been used with 5.2 / 5.3 toolchains; align `sdkconfig` with your IDF install).
- **CMake** / **Ninja** (via ESP-IDF).

## Build, flash, monitor

```bash
# Load ESP-IDF environment (path may differ on your machine)
. $HOME/esp-idf/export.sh

cd EITWatch_Firmware   # or your clone path

idf.py set-target esp32s3   # or esp32c3, etc., to match hardware

idf.py build
idf.py -p /dev/tty.usbmodemXXXX flash monitor
```

Artifact name follows the CMake project: e.g. **`build/EITWatch_Firmware.bin`**.

## Configuration notes

- **`sdkconfig`**: chip target, flash size, Wi-Fi/Ethernet options, log level.
- **`wireless.h`**: `WIFI_SSID`, `WIFI_PASSWORD`, `PORT_NUM`, broadcast vs future unicast edits in `wireless.c`.
- **`Middle_Ware/hardware.h`**: SPI pins and chip selects for your board.

## License / contributing

Add a `LICENSE` and contribution guidelines if this repository is public or shared; none are defined in-tree by default.
