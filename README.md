# Weather Station

This project is a battery-powered environmental monitoring system built around an ESP32 and BME280 sensor. It measures temperature, relative humidity, atmospheric pressure, Wi-Fi signal strength, and battery state, then transmits the measurements over Wi-Fi to a self-hosted server for storage and visualization.

The project was designed as a modular sensor platform rather than a single-purpose weather station. The ESP32 firmware handles sensor acquisition, battery monitoring, network communication, and power management, while a separate server stack receives and stores the measurements and provides historical visualization.
## Prototype

### Closed enclosure

![Closed weather station prototype](images/v1_closed.jpg)

### Open enclosure

![Weather station internal components](images/v1_open.jpg)


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
* MT3608 boost converter
* Resistor voltage divider for battery monitoring
* 3D-printed enclosure

The BME280 communicates with the ESP32 over I2C.

## Data Collected

The ESP32 currently sends:

* Temperature
* Relative humidity
* Atmospheric pressure
* Wi-Fi RSSI
* Battery voltage
* Estimated battery percentage

## Firmware

The current firmware is located in:

```text
src/main.cpp
```

The firmware follows a simple measurement cycle:

1. Wake from deep sleep.
2. Initialize the sensor hardware.
3. Connect to Wi-Fi.
4. Collect environmental and battery data.
5. Send the data to the server using HTTP and JSON.
6. Return to deep sleep.

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

## Power Management

The weather station is powered by a single 18650 lithium-ion cell. The battery voltage is boosted by an MT3608 boost converter, which provides the regulated supply used by the weather station electronics.

Because continuous operation of the ESP32 and Wi-Fi radio would result in unnecessarily high power consumption, the firmware uses the ESP32's deep-sleep mode between measurements.

During each measurement cycle, the ESP32 wakes, initializes the BME280, connects to Wi-Fi, collects sensor and battery measurements, transmits the resulting JSON payload to the server, and returns to deep sleep. The current firmware performs this cycle approximately once per minute.

### Battery Voltage Measurement

Battery voltage is monitored through an ESP32 ADC input. A resistor voltage divider scales the battery voltage before it reaches the ADC.

The firmware reads the ADC multiple times and averages the samples to reduce measurement noise. The measured ADC voltage is then converted back to the estimated battery-terminal voltage using the voltage-divider ratio.

### Battery State-of-Charge Estimation

Lithium-ion battery voltage does not vary linearly with remaining capacity. For this reason, battery percentage is not calculated using a simple linear relationship between maximum and minimum cell voltage.

Instead, the firmware uses a piecewise approximation of the lithium-ion discharge curve to estimate state of charge from the measured battery voltage. This provides a more useful estimate than a linear voltage-to-percentage conversion, although it remains an approximation rather than a direct measurement of remaining capacity.

## Server Side

Sensor data is sent as JSON to a server endpoint.

The server stack is based on:

* FastAPI
* InfluxDB
* Grafana

FastAPI receives HTTP POST requests from the ESP32 and writes the measurements to InfluxDB. Grafana queries the stored time-series data and provides visualization of the measurements over time.

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

## Current Status

The weather station is operational and has been tested over multi-day periods.

The core firmware, battery monitoring, sensor acquisition, network transmission, data storage, and visualization pipeline are functional.

Further work may include improvements to battery state-of-charge estimation, hardware packaging, documentation, and expansion of the modular sensor platform.
