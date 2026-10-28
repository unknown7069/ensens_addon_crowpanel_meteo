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

### Quick build check from CLI

To verify that the project still builds without opening VSCode, run:

```powershell
pwsh -ExecutionPolicy Bypass -File scripts/test_build.ps1
```

Add `-Clean` if you need to start from a pristine `idf.py fullclean` before building.


## Simplified dashboard

The main dashboard shows a large clock and full date, current outside and inside
temperatures, the next overnight low, and the following daytime high. Indoor
sensor details and history remain on the Status tab; tap the inside temperature
to open it. Configuration remains on the Settings tab.

Forecast temperatures use the [OpenWeather 3-hour forecast](https://openweathermap.org/api/forecast5).
The low is the coldest forecast point in the next 18:00-06:00 period, and the high
is the warmest point in the following 06:00-18:00 period, using the location's
timezone. Before 06:00, the low covers the remaining current night and the high
covers the coming day. These are estimates from three-hour samples. Missing
forecast data shows `--`. All four temperatures follow the configured unit.

Build with `powershell -ExecutionPolicy Bypass -File scripts/build.ps1`. To run
the forecast boundary tests with a host C++ compiler:

```powershell
g++ -std=c++17 -Isources tests/weather_forecast_test.cpp -o build/weather_forecast_test.exe
./build/weather_forecast_test.exe
```

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
