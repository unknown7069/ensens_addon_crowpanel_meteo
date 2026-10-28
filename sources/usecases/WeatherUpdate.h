#pragma once

#include "entities/Weather.h"
#include "entities/CurrentTime.h"
#include "entities/BM8563.h"
#include "entities/timestamp.h"
#include "entities/ui/Dashboard/Dashboard.h"
#include "esp_heap_caps.h"
#include <ctime>
#include <new>

namespace UseCases
{
class WeatherUpdate
{
    static constexpr char TAG[] = "WeatherUpdate";

    time_t         curTimestamp = 0;
    Weather::Data* weatherInfo = [] {
        void* memory = heap_caps_malloc(sizeof(Weather::Data), MALLOC_CAP_SPIRAM);
        return memory ? new (memory) Weather::Data{} : nullptr;
    }();

    WeatherUpdate()                           = default;
    WeatherUpdate(const WeatherUpdate&)       = delete;
    WeatherUpdate& operator=(const WeatherUpdate&) = delete;

    void applyValues()
    {
        if (!weatherInfo || weatherInfo->timestamp == 0)
            return;
        if (CurrentTime::instance().isTimeSet())
        {
            time(&curTimestamp);
            BM8563::instance().setUnixTimeStamp(curTimestamp);
        }
        CurrentTime::instance().setTimezoneOffset(weatherInfo->timestampOffset);
        Dashboard::instance().updateTimeLabel(static_cast<uint32_t>(curTimestamp),
                                              weatherInfo->timestampOffset);
        Dashboard::instance().updateOutsideTemperature(weatherInfo->temperature);
        Dashboard::instance().updateOutsideHumidity(weatherInfo->humidity);
        Dashboard::instance().updateOutsideWindSpeed(weatherInfo->windSpeed);
        Dashboard::instance().updateOutsideFeelsLike(weatherInfo->feelsLike);
        Dashboard::instance().updateOutsidePressure(weatherInfo->pressure);
        Dashboard::instance().updateOutsideDailyHigh(weatherInfo->nextDayHigh);
        Dashboard::instance().updateOutsideDailyLow(weatherInfo->nextNightLow);
        Dashboard::instance().updateOutsidePrecipitation(weatherInfo->precipitation);
        Dashboard::instance().updateForecast(weatherInfo->forecast, weatherInfo->icon);
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
        if (!weatherInfo || !Weather::instance().getCurrentWeather(weatherInfo))
        {
            return false;
        }
        // Keep current readings available if the forecast fails. Missing
        // forecasts display placeholders rather than current-weather min/max.
        Weather::instance().getForecast(weatherInfo);
        applyValues();
        return true;
    }

    void refresh()
    {
        applyValues();
    }
};
} // namespace UseCases
