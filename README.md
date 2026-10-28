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

### Build from CLI

To verify that the project still builds without opening VSCode, run:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
```

Add `-Clean` if you need to start from a pristine `idf.py fullclean` before building.


## Warm Horizon dashboard

The 800 × 480 home screen uses warm ivory, charcoal text, terracotta accents,
and a large 12-hour clock with AM/PM and a full weekday/date. Current outdoor
temperature, weather conditions and feels-like temperature sit beside the clock.
Indoor temperature and humidity sit below it.

The lower band shows the next 24 hours as eight three-hour forecast samples,
with day/night weather icons and a temperature trend. A divider and weekday
mark the first sample after midnight. Past samples roll off as the clock
advances. Times use the configured weather location's timezone; fractional-hour
timezones retain the forecast's actual minutes. Temperatures, including
feels-like and forecast values, follow the configured Celsius/Fahrenheit unit.

Tap the indoor readings to open sensor details and the last 24 hours of history; use Home
to return. The top-right gear opens the existing configuration screen; its
back button returns to the clock. The Wi-Fi icon dims when disconnected.
Missing forecasts show placeholders and "FORECAST UNAVAILABLE".

Sensor details use six large cards for temperature, humidity, pressure, CO2,
VOC and air quality, with a rolling 24-hour sparkline for each. Charts run from
24 hours ago on the left to now on the right, keeping the latest reading in each
half-hour bucket. They retain the previous day's readings at midnight and age
out old values every minute even when the sensor is offline. Missing readings
remain gaps. History is held in memory and fills as readings arrive after a
restart. Settings uses the same
ivory/oatmeal surfaces, charcoal text and terracotta highlights. Its sidebar
contains Units, Date & time, Brightness, Location and Wi-Fi. Dropdown lists,
calendar, switches, slider, network rows and the on-screen keyboard share the
palette. Date & time scrolls to the calendar; Set time applies the time, while
tapping a calendar day applies the date. When editing a city or Wi-Fi password,
the settings pane shrinks to keep the active field above the keyboard.

The approved mockup is in `design/mockups/04-warm-horizon-24h.png`. A render
of the actual LVGL implementation with sample readings is in
`design/implemented/warm-horizon-800x480.png`.
Additional rendered examples are `warm-sensors.png` and `warm-settings-*.png`
in that same folder. These use sample values; the settings previews share the
firmware's view factories and palette, with hardware callbacks excluded.

Forecast temperatures use the [OpenWeather 3-hour forecast](https://openweathermap.org/api/forecast5).
The low is the coldest forecast point in the next 18:00-06:00 period, and the high
is the warmest point in the following 06:00-18:00 period, using the location's
timezone. Before 06:00, the low covers the remaining current night and the high
covers the coming day. These are estimates from three-hour samples. Missing
forecast data shows `--`. All four temperatures follow the configured unit.

After building, connect the panel by USB and use VS Code's ESP-IDF flash/monitor
action, or run this in an ESP-IDF terminal from the project directory (replace
`COM5` with the panel's serial port):

```powershell
idf.py -p COM5 flash monitor
```

The application binary is `build/esp32_lvgl_ensens_crowpanel.bin`; standard
ESP-IDF flashing also uses the generated bootloader and partition table.
Exit the monitor with Ctrl+].

For the first hardware check, verify the clock and current readings, all eight
forecast columns and night icons, then open and return from sensor details and
settings. Switching temperature units should refresh both current and forecast
readings immediately. Hardware rendering and touch still need to be checked on
the physical panel.

Also check switching settings pages, opening dropdowns, manual brightness after
turning automatic brightness off, scrolling to the calendar, and opening and
closing the city/password keyboard. Saved units, location and Wi-Fi behavior use
the existing settings handlers.

To run the forecast boundary tests alone with a host C++ compiler:

```powershell
g++ -std=c++17 -Isources tests/weather_forecast_test.cpp -o build/weather_forecast_test.exe
./build/weather_forecast_test.exe
```

The desktop checks in `tests/preview` cover rolling indoor history across
midnight, bucket wraparound, exact 24-hour expiry, missing readings and unit
metadata. They also exercise the real forecast JSON parser,
render the actual LVGL view at 800 × 480, check all 1,440 clock minutes for
overlap, and test unit conversion, missing data, midnight markers and navigation.
With CMake, native GCC and Ninja available:

```powershell
cmake -S tests/preview -B build/preview -G Ninja -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
cmake --build build/preview
ctest --test-dir build/preview --output-on-failure
```

The layout test writes `dashboard.bmp`, `dashboard-night-fahrenheit.bmp`,
`dashboard-unavailable.bmp`, `dashboard-sensors.bmp`, and `dashboard-settings-*.bmp`
to `build/preview`. Settings checks cover all sidebar pages, dropdown popup
colors, calendar scrolling, the keyboard and returning Home. This uses a stub only for ESP32
allocation; rendering uses the project's managed LVGL 8.4 sources and the same
view, icons and fonts as the firmware. It requires the managed dependencies to
have been downloaded by an ESP-IDF build.

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
