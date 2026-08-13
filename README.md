# weather-station
# Weather Station

A modular ESP32-based sensor platform for collecting environmental and system data, transmitting it to a server, and visualizing measurements over time.

The current implementation functions as a battery-powered weather station. It measures temperature, humidity, atmospheric pressure, Wi-Fi signal strength, battery voltage, and estimated battery state of charge. The ESP32 sends this data over Wi-Fi to a server for storage and visualization.

## Project Goals

The main goal of this project is to build a reusable sensor platform rather than a single-purpose device. The weather station is the first implementation, but the same basic architecture can be adapted to other sensors and monitoring applications.

The system is designed around a simple cycle:

1. Wake from deep sleep.
2. Initialize the sensor hardware.
3. Connect to Wi-Fi.
4. Collect environmental and battery data.
5. Send the data to the server using HTTP and JSON.
6. Return to deep sleep to reduce power consumption.

## System Architecture

```text
BME280 ──I2C──► ESP32 ──Wi-Fi / HTTP──► FastAPI ──► InfluxDB ──► Grafana
                   ▲
                   │
            Battery Monitoring
```

## Hardware

The current weather station uses:

* ESP32 development board
* BME280 temperature, humidity, and pressure sensor
* 18650 lithium-ion battery
* TP4056 lithium-ion charging module
* MT3608 boost converter
* Resistor voltage divider for battery monitoring
* 3D-printed enclosure

The BME280 communicates with the ESP32 over I2C.

The battery voltage is measured using an ADC input on the ESP32. A resistor divider reduces the battery voltage to a safe level for the ADC, and multiple samples are averaged to reduce measurement noise.

## Data Collected

The ESP32 currently sends:

* Temperature
* Relative humidity
* Atmospheric pressure
* Wi-Fi RSSI
* Battery voltage
* Estimated battery percentage

Battery percentage is estimated from measured cell voltage using a piecewise voltage-to-state-of-charge approximation.

## Firmware

The current firmware is located in:

```text
src/main.cpp
```

The firmware is built and uploaded using PlatformIO.

To build the firmware:

```bash
pio run
```

To upload it to the ESP32:

```bash
pio run -t upload
```

To open the serial monitor:

```bash
pio device monitor
```

## Configuration

Wi-Fi credentials are stored separately from the main source code and are not included in the repository.

Create:

```text
src/secrets.h
```

with:

```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
```

The file is excluded from Git using `.gitignore`.

## Repository Structure

```text
weather-station/
├── src/
│   └── main.cpp
├── include/
├── lib/
├── test/
├── hardware/
│   └── enclosure/
├── images/
├── docs/
├── older versions/
├── platformio.ini
├── .gitignore
└── README.md
```

### `src/`

Contains the current ESP32 firmware.

### `hardware/`

Contains physical design files associated with the weather station, including STL files for the 3D-printed enclosure.

### `older versions/`

Contains firmware and test programs created before the project was placed under Git version control. These files are preserved to show earlier stages of development.

Future revisions are tracked through Git commits instead of additional manually numbered versions.

### `docs/`

Reserved for project documentation, wiring information, setup instructions, and related technical notes.

## Server Side

Sensor data is sent as JSON to a server endpoint.

The server stack is based on:

* FastAPI
* InfluxDB
* Grafana

FastAPI receives the HTTP POST request from the ESP32. The measurements are then written to InfluxDB and visualized over time using Grafana.

## Power Management

The ESP32 spends most of its operating time in deep sleep.

The current firmware wakes approximately once per minute, performs a measurement and transmission cycle, and then returns to deep sleep.

This substantially reduces power consumption compared with leaving the ESP32, Wi-Fi radio, and sensor system continuously active.

## Current Status

The weather station is operational and has been tested over multi-day periods.

The core firmware, battery monitoring, sensor acquisition, network transmission, data storage, and visualization pipeline are functional.

Further work may include improvements to battery state-of-charge estimation, hardware packaging, documentation, and expansion of the modular sensor platform.

