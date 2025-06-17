## Prepare system

 1.Install VSCode.
 2.Install ESP-IDF Extension - Develop and debug applications for Espressif chips with ESP-IDF.
 3.Install ESP-IDF v5.4.1 (via ESP-IDF Extension).
 4.Open ENSENS_METEO project:
 - from "root" directory - write "code ." from CLI;
 - from "VSCode" - select "Open Folder..." from Menu.
 5.Configure VSCode "settings.json" (if you need).

## Build and Flash ENSENS_METEO project
 1.Сonnect your panel with a type-C cable to the PC port.
 2.Into VSCode:
 - select current ESP-IDF v5.4.1 version;
 - select ESP-IDF Flash Method - UART;
 - select Port to Use - your active COM port;
 - select Espressif Device Target- esp32s3.
 3.Into VSCode:
 - click "Build, Flash and Monitor".



# Indoor Screen for Meteo project

## Overview

It is a sensor dashboard for the Elecrow Panel based on ESP32-S3 and built with LVGL.
Add-On sensor need to connect to the Panel at the back.
The jumper (at the back) must be moved to the desired position (WM).

It features a simple touch-based interface with two screens:

### 1. Settings Screen
- Configure Units for display (Temperature + Pressure)
- Configure Data and Time in manual mode (to write to the device memory)
- Configure Wi-Fi (Access Point + client mode)
- Adjust screen brightness
- Select city manually

### 2. Indoor Display Screen
- Current day of the week
- Current date and time
- Current zodiac sign
- Current temperature value
- Current humidity value
- Current dew point value
- Current pressure value
- Pressure graph for one day
- Current IAQ value
- Current CO2 value
- Current VOC value
- Trend value for measurable parameters

### Data Sources
- Add-on Indoor sensor.
---

# Forecast Screen for Meteo project

## Overview

It is a weather dashboard for the Elecrow Panel based on ESP32-S3 and built with LVGL.

It features a simple touch-based interface with two screens:

### 1. Settings Screen
- Configure Units for display (Temperature + Pressure)
- Configure Data and Time in manual mode (to write to the device memory)
- Configure Wi-Fi (Access Point + client mode)
- Adjust screen brightness
- Select city manually

### 2. Forecast Display Screen
- Current weather
- Temperature
- Feels like
- Cloud coverage
- Humidity
- Wind speed
- Atmospheric pressure
- Sunrise and sunset time
- Today’s forecast: 4 timepoints (every 3 hours)
- 4-day forecast: day and night temperatures

### Data Sources
- Weather data: [openweather.com](https://openweather.com)
- City auto-detection (optional): [ip-api.com](http://ip-api.com)
- Time synchronization: [pool.ntp.org](https://www.pool.ntp.org)

---
