# Golf Cart Battery Monitor

A battery monitoring system for golf carts that connects to BAT-BMS over Bluetooth LE and displays battery status on a 240x240 TFT display.

## Hardware

- ESP32-2424S012N/C module:
  - ESP32-C3-MINI-1U core (Single Core MCU)
  - 400KB SRAM, 384KB ROM, 4MB Flash
  - 160MHz max frequency
  - Integrated WiFi and Bluetooth
- 1.28" IPS LCD Display:
  - 240x240 resolution
  - 16-bit color
  - GC9A01 driver

## Features

- Connects to BAT-BMS over Bluetooth LE
- Real-time display of:
  - Battery State of Charge (%)
  - Current (A)
  - Voltage (V)
  - Power (W)
  - Time to full charge estimation
- Automatic reconnection on connection loss
- Moving average filtering for stable readings
- Charge time estimation during charging

## Building

This project uses PlatformIO for building. To build:

1. Install PlatformIO
2. Clone this repository
3. Run `pio run` to build
4. Run `pio run --target upload` to flash

## Configuration

- Default BMS address is set in `main.cpp`
- Display uses hardware SPI
- For development/testing, enable emulator mode with `-D USE_EMULATOR` flag

## Dependencies

- LVGL for UI
- LovyanGFX for display driver
- Arduino framework
- ESP32 BLE libraries

## License

MIT License