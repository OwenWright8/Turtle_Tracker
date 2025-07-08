# 🐢 TinyZero Accelerometer Data Logger

## Overview

This project is a **low-power accelerometer and temperature data logger** built using the **TinyCircuits TinyZero** platform. It records motion and environmental data to a microSD card using precise timestamps from a real-time clock (RTC). The goal is to log animal movement (e.g. turtles) or environmental conditions in remote, battery-powered conditions.

---

## 🔧 Hardware Components

| Component                             | Purpose |
|--------------------------------------|---------|
| **TinyCircuits TinyZero**            | Main microcontroller (ATSAMD21G18) |
| **TinyCircuits TinyShield MicroSD**  | Stores CSV log data and status logs |
| **TinyCircuits TinyShield RTC**      | Keeps accurate timestamps (DS1339) |
| **TinyZero Built-in BMA250**         | 3-axis accelerometer & temperature sensor |
| **3.7V LiPo Battery**                | Powers the device in the field |
| **LED (onboard)**                    | Visual feedback for status & errors |

---

## 📁 File Output Structure

### `/d/logMMDD.csv`
Main data log file for the day (one per calendar day).
```
Date,Time,X,Y,Z,Temp(C)
2025-07-08,11:06:03,4,53,245,27.50
```

### `/d/startup.log`
Each startup event is logged with timestamps.
```
2025-07-08,11:06:01,STARTUP,Accelerometer initialized
```

### `/d/errors.log`
Any detected error (e.g. missing sensor, SD failure) is logged with a timestamp.
```
2025-07-08,11:06:03,ERROR,BMA250 NOT DETECTED
```

---

## 📦 Features

- ✅ Accelerometer + temperature logging every 10 seconds  
- ✅ Real-time timestamps via RTC  
- ✅ Automatic daily file creation  
- ✅ Onboard LED blink diagnostics  
- ✅ Startup and error logs separated from data  
- ✅ Calibration offsets applied for consistent readings  

---

## 💡 LED Blink Codes

| Blink Pattern                 | Meaning |
|------------------------------|---------|
| **3 short blinks**           | Device started (`setup()` entered) |
| **1 long blink**             | Wire (I2C) initialized |
| **2 long blinks**            | RTC initialized |
| **3 long blinks**            | SD card initialized |
| **4 long blinks**            | `/d` directory created |
| **3 fast blinks**            | Accelerometer initialized |
| **1 short blink (every 10s)**| New log entry saved |
| **5 long blinks** (looping)  | Fatal error (e.g. SD/RTC/sensor failure) |

---

## 📌 Setup Instructions

1. Install the latest Arduino IDE  
2. Install the TinyCircuits board package  
3. Load the code onto your TinyZero  
4. Insert a formatted microSD card  
5. Power with USB or 3.7V LiPo battery  

---

## 🧪 Example Use Case

- Track turtle activity in a natural habitat  
- Log movement patterns and resting durations  
- Collect temperature exposure over time  
- Operate fully unattended for days

---

## 📞 Support / Troubleshooting

If you're having issues with logging, power, or sensor readings:

- ✅ Check the `/d/errors.log` file for diagnostics  
- ✅ Use the LED blink pattern to identify where it stopped  
- ✅ Ensure battery is charged and SD card is working
