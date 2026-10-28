#include "entities/WeatherForecastParser.h"

#include <cassert>
#include <cstring>
#include <string>

int main()
{
    constexpr int64_t midnight = 20000LL * 86400;
    constexpr int64_t hour = 3600;
    constexpr int32_t offset = -6 * hour;
    const int64_t now = midnight + 8 * hour + 42 * 60;
    std::string json = "{\"list\":[";
    for (int i = 0; i < 24; ++i)
    {
        if (i) json += ',';
        json += "{\"dt\":" + std::to_string(midnight + (9 + 3 * i) * hour - offset) +
                ",\"main\":{\"temp\":" + std::to_string(6 - i) + "}," +
                "\"weather\":[{\"icon\":\"" + (i < 4 ? "02d" : "01n") + "\"}]}";
    }
    json += "]}";
    WeatherForecast forecast;
    assert(parseWeatherForecast(json.c_str(), json.size(), now, offset, forecast));
    assert(forecast.count == 24);
    const auto day = forecast.next24Hours(now);
    assert(day.front().timestamp == midnight + 9 * hour);
    assert(day.back().timestamp == midnight + 30 * hour);
    assert(day.back().temperature == -1);
    assert(std::strcmp(day[0].icon, "02d") == 0);
    assert(std::strcmp(day[6].icon, "01n") == 0);

    // Drop past points as the kitchen clock advances, even without a fetch.
    const auto later = forecast.next24Hours(midnight + 12 * hour);
    assert(later.front().timestamp == midnight + 15 * hour);
    assert(later.back().timestamp == midnight + 36 * hour);
    for (const auto& point : forecast.next24Hours(midnight + 100 * hour))
        assert(point.timestamp == 0 && std::isnan(point.temperature));

    const std::string noIcon = "{\"list\":[{\"dt\":" + std::to_string(now + hour - offset) +
                               ",\"main\":{\"temp\":-5.5}}]}";
    assert(parseWeatherForecast(noIcon.c_str(), noIcon.size(), now, offset, forecast));
    assert(forecast.count == 1 && forecast.points[0].temperature == -5.5f);
    assert(forecast.points[0].icon[0] == '\0');

    // An invalid later item must not leave earlier valid data on screen.
    const std::string broken = "{\"list\":[{\"dt\":" + std::to_string(now + hour - offset) +
                               ",\"main\":{\"temp\":-5}},{\"dt\":2}]}";
    assert(!parseWeatherForecast(broken.c_str(), broken.size(), now, offset, forecast));
    assert(forecast.count == 0 && std::isnan(forecast.nightLow));
    assert(!parseWeatherForecast("{", 1, now, offset, forecast));
    assert(!parseWeatherForecast("{\"list\":[]}", 11, now, offset, forecast));
    assert(!parseWeatherForecast(nullptr, 0, now, offset, forecast));

    // Fractional timezones preserve the actual local forecast minute.
    assert(parseWeatherForecast(json.c_str(), json.size(), midnight, 19800, forecast));
    assert(forecast.points[0].timestamp % hour == 1800);
}
