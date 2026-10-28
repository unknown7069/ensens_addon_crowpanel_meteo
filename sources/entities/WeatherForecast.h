#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

struct WeatherForecastPoint {
    int64_t timestamp = 0; // Location-local epoch (UTC + location offset).
    float temperature = std::numeric_limits<float>::quiet_NaN(); // Celsius.
    char icon[4] = {};
};

// Use location-local timestamps. Before 06:00 use the remaining current night;
// otherwise use the upcoming 18:00-06:00 night and following 06:00-18:00 day.
struct WeatherForecast {
    static constexpr size_t Capacity = 24;
    static constexpr size_t DisplayPoints = 8;
    std::array<WeatherForecastPoint, Capacity> points{};
    size_t count = 0;
    int64_t referenceTime;
    int64_t nightStart;
    int64_t dayStart;
    int64_t dayEnd;
    float nightLow = std::numeric_limits<float>::quiet_NaN();
    float dayHigh = std::numeric_limits<float>::quiet_NaN();

    explicit WeatherForecast(int64_t localNow = 0) : referenceTime(localNow)
    {
        const int64_t midnight = localNow - localNow % 86400;
        nightStart = localNow < midnight + 6 * 3600
            ? midnight - 6 * 3600 : midnight + 18 * 3600;
        dayStart = nightStart + 12 * 3600;
        dayEnd = dayStart + 12 * 3600;
    }

    void add(int64_t localTimestamp, float temperature, const char* icon = nullptr)
    {
        if (!std::isfinite(temperature))
            return;
        if (localTimestamp <= referenceTime)
            return;
        // OpenWeather supplies chronological three-hour samples. Ignore
        // duplicates/out-of-order samples rather than repeating a time slot.
        if (count && localTimestamp <= points[count - 1].timestamp)
            return;
        if (count < points.size())
        {
            auto& point = points[count++];
            point.timestamp = localTimestamp;
            point.temperature = temperature;
            if (icon && std::strlen(icon) == 3)
                std::memcpy(point.icon, icon, 3);
        }
        if (localTimestamp >= nightStart && localTimestamp < dayStart)
            nightLow = std::isfinite(nightLow) ? std::min(nightLow, temperature) : temperature;
        if (localTimestamp >= dayStart && localTimestamp < dayEnd)
            dayHigh = std::isfinite(dayHigh) ? std::max(dayHigh, temperature) : temperature;
    }

    std::array<WeatherForecastPoint, DisplayPoints> next24Hours(int64_t localNow) const
    {
        std::array<WeatherForecastPoint, DisplayPoints> next{};
        size_t index = 0;
        for (size_t i = 0; i < count && index < next.size(); ++i)
        {
            const auto& point = points[i];
            if (point.timestamp > localNow && point.timestamp <= localNow + 86400)
                next[index++] = point;
        }
        return next;
    }
};
