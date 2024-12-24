# Golf Cart Battery Monitor

A battery monitoring system for golf carts that connects to BAT-BMS over Bluetooth LE and displays battery status on a 240x240 TFT display.

## Hardware

- ESP32-C3-MINI-1-N4 module (ESP32-C3-DevKitM-1 development board)
- 240x240 TFT LCD Display (ST7789 driver)
- Power supply: 5V via USB or 3.3V direct

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