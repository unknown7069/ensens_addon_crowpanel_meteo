#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

// Use location-local timestamps. Before 06:00 use the remaining current night;
// otherwise use the upcoming 18:00-06:00 night and following 06:00-18:00 day.
struct WeatherForecast {
    int64_t nightStart;
    int64_t dayStart;
    int64_t dayEnd;
    float nightLow = std::numeric_limits<float>::quiet_NaN();
    float dayHigh = std::numeric_limits<float>::quiet_NaN();

    explicit WeatherForecast(int64_t localNow)
    {
        const int64_t midnight = localNow - localNow % 86400;
        nightStart = localNow < midnight + 6 * 3600
            ? midnight - 6 * 3600 : midnight + 18 * 3600;
        dayStart = nightStart + 12 * 3600;
        dayEnd = dayStart + 12 * 3600;
    }

    void add(int64_t localTimestamp, float temperature)
    {
        if (!std::isfinite(temperature))
            return;
        if (localTimestamp >= nightStart && localTimestamp < dayStart)
            nightLow = std::isfinite(nightLow) ? std::min(nightLow, temperature) : temperature;
        if (localTimestamp >= dayStart && localTimestamp < dayEnd)
            dayHigh = std::isfinite(dayHigh) ? std::max(dayHigh, temperature) : temperature;
    }
};
