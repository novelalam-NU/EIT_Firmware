#!/usr/bin/env python3
import argparse
import re
import time
from collections import deque

import matplotlib.pyplot as plt
from matplotlib.widgets import Button, TextBox  # Added TextBox
import serial
from serial.tools import list_ports

ADC_PATTERN = re.compile(r"ADC:\s*(\d+)")

def select_port(user_port: str | None) -> str:
    if user_port:
        return user_port
    ports = [p.device for p in list_ports.comports()]
    for pattern in ("/dev/cu.usbmodem", "/dev/tty.usbmodem", "/dev/cu.usbserial", "/dev/tty.usbserial"):
        for port in ports:
            if port.startswith(pattern):
                return port
    if ports:
        return ports[0]
    raise RuntimeError("No serial ports found. Connect device or pass --port explicitly.")

def open_serial(port: str, baud: int) -> serial.Serial:
    return serial.Serial(port, baud, timeout=1)

def main() -> None:
    parser = argparse.ArgumentParser(description="Live plot ADC values from ESP serial output")
    parser.add_argument("--port", default=None, help="Serial port")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate")
    parser.add_argument("--window", type=int, default=100, help="Initial samples to show")
    parser.add_argument("--headless", action="store_true", help="Print values without plot")
    args = parser.parse_args()

    port = select_port(args.port)
    print(f"Using serial port: {port}")
    
    while True:
        try:
            ser = open_serial(port, args.baud)
            break
        except serial.SerialException as exc:
            print(f"Waiting for serial port ({exc})")
            time.sleep(0.5)

    # Increased maxlen to accommodate larger manual window sizes
    current_window = args.window
    x_vals = deque(maxlen=5000) 
    y_vals = deque(maxlen=5000)
    sample_idx = 0

    if not args.headless:
        plt.ion()
        fig, ax = plt.subplots(figsize=(12, 6))
        plt.subplots_adjust(bottom=0.25) # More space for text box and buttons
        
        # 'o-' creates a line with dots. markersize and markeredgewidth make them "bigger"
        line, = ax.plot([], [], 'o-', lw=1.5, color='#2c3e50', 
                        markersize=6, 
                        markerfacecolor='#e74c3c', 
                        markeredgecolor='white')
        
        ax.set_title("Live ADC Data")
        ax.set_xlabel("Sample Index")
        ax.set_ylabel("ADC Value")
        ax.grid(True, linestyle=':', alpha=0.7)

        # --- UI Interaction Logic ---
        def update_window(text):
            nonlocal current_window
            try:
                new_val = int(text)
                if new_val > 0:
                    current_window = new_val
                    print(f"Window updated to: {new_val}")
            except ValueError:
                pass # Ignore non-integer input

        # Text Box Position [left, bottom, width, height]
        ax_box = plt.axes([0.2, 0.05, 0.15, 0.075])
        text_box = TextBox(ax_box, 'Window Size: ', initial=str(current_window))
        text_box.on_submit(update_window)

        # Preset Buttons
        ax_500 = plt.axes([0.5, 0.05, 0.1, 0.075])
        ax_1000 = plt.axes([0.62, 0.05, 0.1, 0.075])
        btn_500 = Button(ax_500, '500')
        btn_1000 = Button(ax_1000, '1000')

        btn_500.on_clicked(lambda x: update_window("500"))
        btn_1000.on_clicked(lambda x: update_window("1000"))

    print("Listening for ADC lines...")

    try:
        while True:
            try:
                raw = ser.readline().decode("utf-8", errors="ignore").strip()
            except serial.SerialException:
                ser.close()
                time.sleep(0.5)
                ser = open_serial(port, args.baud)
                continue

            if not raw: continue
            match = ADC_PATTERN.search(raw)
            if not match: continue

            value = int(match.group(1))
            x_vals.append(sample_idx)
            y_vals.append(value)
            sample_idx += 1

            if args.headless:
                if sample_idx % 100 == 0: print(f"ADC: {value}")
                continue

            # Slice data for the sliding window
            plot_x = list(x_vals)[-current_window:]
            plot_y = list(y_vals)[-current_window:]
            
            line.set_data(plot_x, plot_y)
            ax.relim()
            ax.autoscale_view()
            
            # Keep the view focused on the sliding window
            if plot_x:
                if len(plot_x) > 1 and plot_x[0] != plot_x[-1]:
                    ax.set_xlim(plot_x[0], plot_x[-1])
                else:
                    center = plot_x[0]
                    ax.set_xlim(center - 1, center + 1)

            plt.pause(0.001)

    except KeyboardInterrupt:
        pass
    finally:
        ser.close()
        if not args.headless:
            plt.ioff()
            plt.show()

if __name__ == "__main__":
    main()