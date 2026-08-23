#pragma once

#include "entities/Weather.h"
#include "entities/CurrentTime.h"
#include "entities/BM8563.h"
#include "entities/timestamp.h"
#include "entities/ui/Dashboard/Dashboard.h"
#include "esp_heap_caps.h"
#include <ctime>

namespace UseCases
{
class WeatherUpdate
{
    static constexpr char Tag[] = "WeatherUpdate";

    time_t         curTimestamp = 0;
    Weather::Data* weatherInfo =
        static_cast<Weather::Data*>(heap_caps_calloc(1, sizeof(Weather::Data), MALLOC_CAP_SPIRAM));

    WeatherUpdate()                           = default;
    WeatherUpdate(const WeatherUpdate&)       = delete;
    WeatherUpdate& operator=(const WeatherUpdate&) = delete;

    void applyValues()
    {
        if (weatherInfo->timestamp == 0)
            return;
        if (CurrentTime::instance().isTimeSet())
        {
            time(&curTimestamp);
            BM8563::instance().setUnixTimeStamp(curTimestamp);
        }
        CurrentTime::instance().setTimezoneOffset(weatherInfo->timestampOffset);
        TimeStamp::instance().is_sync_current_time = 1;
        Dashboard::instance().updateTimeLabel(static_cast<uint32_t>(curTimestamp),
                                              weatherInfo->timestampOffset);
        Dashboard::instance().updateOutsideTemperature(weatherInfo->temperature);
        Dashboard::instance().updateOutsideHumidity(weatherInfo->humidity);
        Dashboard::instance().updateOutsideWindSpeed(weatherInfo->windSpeed);
        Dashboard::instance().updateOutsideFeelsLike(weatherInfo->feelsLike);
        Dashboard::instance().updateOutsidePressure(weatherInfo->pressure);
        Dashboard::instance().updateOutsideDailyHigh(weatherInfo->tempMax);
        Dashboard::instance().updateOutsideDailyLow(weatherInfo->tempMin);
        Dashboard::instance().updateOutsidePrecipitation(weatherInfo->precipitation);
        WifiScreen::instance().setLocation(*weatherInfo);
    }

public:
    static WeatherUpdate& instance()
    {
        static WeatherUpdate instance;
        return instance;
    }

    bool update()
    {
        if (!Weather::instance().getCurrentWeather(weatherInfo))
        {
            return false;
        }
        applyValues();
        return true;
    }

    void refresh()
    {
        applyValues();
    }
};
} // namespace UseCases
