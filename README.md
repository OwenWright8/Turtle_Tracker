# Turtle Accelerometer Logger (TinyZero + BMA250 + RTC + microSD)

A low-power turtle movement logger that records 3-axis accelerometer data (X/Y/Z) plus onboard temperature to a daily CSV file on a microSD card, with timestamping from an RTC and separate startup/error logs.

This project is designed for field deployments where you want consistent time-stamped motion data with minimal power use.

---

## Hardware

**Main components**
- **TinyCircuits TinyZero** (ATSAMD21G18)
- **TinyZero built-in BMA250** accelerometer (3-axis + temperature)
- **TinyCircuits TinyShield MicroSD** (data storage)
- **TinyCircuits TinyShield RTC** (DS1339 — keeps time for timestamps)
- **3.7V LiPo battery**

**microSD wiring note**
- This sketch uses `const int chipSelect = 10;`
- Ensure your TinyShield MicroSD CS pin is wired/mapped to **D10** (or change `chipSelect` to match your build).

---

## Libraries Used

- `Wire.h` (I2C)
- `SPI.h` + `SD.h` (microSD logging)
- `RTClib.h` (RTC interface)
- `BMA250.h` (TinyCircuits BMA250 driver)
- `ArduinoLowPower.h` (sleep to reduce battery drain)

Install these through the Arduino Library Manager when available, or via the TinyCircuits library releases.

---

## What It Logs

Each sample logs:
- **Date** (YYYY-MM-DD)
- **Time** (HH:MM:SS)
- **X, Y, Z** accelerometer readings (raw integer values from BMA250)
- **Temp(C)** estimated temperature derived from the BMA250 raw temperature register:
  - `temp = rawTemp * 0.5 + 24.0`

### CSV Header
`Date,Time,X,Y,Z,Temp(C)`

---

## File & Folder Structure on SD Card

On boot, the sketch ensures a `/d` directory exists and writes three kinds of files:

### 1) Daily data CSV (movement + temp)
- **Path format:** `/d/logMMDD.csv`
- Example (Jan 13): `/d/log0113.csv`
- If the file is new, it writes the CSV header once.

### 2) Startup log
- **Path:** `/d/startup.log`
- Contains timestamped status messages for successful initialization steps.

### 3) Error log
- **Path:** `/d/errors.log`
- Contains timestamped error messages (e.g., file open failures, sensor missing).

---

## Sampling Interval / Power Behavior

Each loop:
1. Reads BMA250 accel + temperature
2. Prints a line to `SerialUSB` (for debugging)
3. Appends one CSV row to the daily log file and flushes it
4. Blinks the onboard LED briefly
5. Sleeps using low power mode:
   - `LowPower.sleep(7000);`

**Important:** there is also a ~150 ms LED blink delay and some logging overhead, so the *effective* interval is a bit more than 7 seconds. The comment in the code notes this is intended to approximate a ~10 second cadence depending on overhead.

---

## LED Blink Codes

The built-in LED is used as a simple “status UI” during setup and fault conditions:

- **3 quick blinks (200 ms):** sketch started
- **1 blink:** `Wire.begin()` done
- **2 blinks:** RTC detected/initialized
- **3 blinks:** SD card initialized
- **4 blinks:** `/d` folder created (only if it didn’t already exist)
- **3 fast blinks (100 ms):** accelerometer initialized
- **Short single blink each loop:** a sample was logged

**Fatal errors:** the sketch uses **5 blinks** and then halts (`while(1);`) for major failures such as:
- RTC not found / not initialized
- SD init failure
- failure to create `/d`
- failure to open startup/error/data logs
- BMA250 not detected (special case below)

---

## Sensor Missing / Fault Handling

After reading the accelerometer, the code checks:

```cpp
if (x == -1 && y == -1 && z == -1) {
  logError("BMA250 NOT DETECTED");
  blinkLED(5, 200);
  while (1);
}
