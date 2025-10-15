#pragma once

#include "adapters/lvgl/ScreenBase.h"
#include "entities/Weather.h"
#include "CurrentTime.h"
#include "esp_heap_caps.h"
#include "entities/Brightness.h"
#ifdef COMMON_DEMO_APP
#include "entities/ui/components/common.h"
#include "entities/ui/weather_screen/CurrentWeatherBody.h"
#include "entities/ui/weather_screen/ForecastTable.h"
#include "entities/ui/weather_screen/Header.h"
#include "entities/ui/weather_screen/TodayForecastTable.h"
#include "timestamp.h"
#include "ui/Dashboard/Dashboard.h"
#else
#include "entities/weather_screen/Header.h"
#include "entities/weather_screen/CurrentWeatherBody.h"
#include "entities/weather_screen/TodayForecastTable.h"
#include "entities/weather_screen/ForecastTable.h"
#endif

class WeatherScreen : public ScreenBase
{
    struct UI {
        Header             header = Header(Header::ShowWifiIconOnly);
        CurrentWeatherBody currentWeatherBody;
        TodayForecastTable todayForecastTable;
        ForecastTable      forecastTable;
        FlexContainer      mainContainer;
    }* ui;

#ifdef COMMON_DEMO_APP
    SensorSettings* sensor_settings_;
    lv_obj_t*       tabview_parent;

    time_t         curTimestamp = 0;
    Weather::Data* forecast_ =
        static_cast<Weather::Data*>(heap_caps_calloc(40, sizeof(Weather::Data), MALLOC_CAP_SPIRAM));
    Weather::Data* weatherInfo =
        static_cast<Weather::Data*>(heap_caps_calloc(1, sizeof(Weather::Data), MALLOC_CAP_SPIRAM));
#endif

public:
    static WeatherScreen& instance()
    {
        static WeatherScreen instance;
        return instance;
    }
#ifdef COMMON_DEMO_APP
    void create(SensorSettings* sensor_settings, tabview_t* tabview_)
    {
        sensor_settings_ = sensor_settings;
        createScreen(tabview_->tab_2);
        ui = new (heap_caps_malloc(sizeof(UI), MALLOC_CAP_SPIRAM)) UI;
        lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        ui->mainContainer.create(screen, LV_PCT(100), LV_PCT(100), LV_FLEX_FLOW_COLUMN);
        ui->mainContainer.align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(ui->mainContainer.get(), LV_OBJ_FLAG_SCROLLABLE);
        ui->header.create(ui->mainContainer);
        ui->currentWeatherBody.create(sensor_settings_, ui->mainContainer.get());
        ui->todayForecastTable.create(sensor_settings_, ui->mainContainer.get());
        ui->forecastTable.create(sensor_settings_, ui->mainContainer.get());
        tabview_parent = tabview_->parent;
    }

    virtual void load()
    {
        if (!screen)
        {
            return;
        }
        lock();
        lv_disp_load_scr(tabview_parent);
        unlock();
    }
#else
    void create()
    {
        createScreen();
        ui = new (heap_caps_malloc(sizeof(UI), MALLOC_CAP_SPIRAM)) UI;
        lv_obj_clear_flag(screen, LV_OBJ_FLAG_SCROLLABLE);
        ui->mainContainer.create(screen, LV_PCT(100), LV_PCT(100), LV_FLEX_FLOW_COLUMN);
        ui->mainContainer.align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        ui->header.create(ui->mainContainer);
        ui->currentWeatherBody.create(ui->mainContainer.get());
        ui->todayForecastTable.create(ui->mainContainer.get());
        ui->forecastTable.create(ui->mainContainer.get());
    }
#endif

    void setCurrentWeather(Weather::Data& data)
    {
        ui->currentWeatherBody.setTemperature(data.temperature);
        Dashboard::instance().updateOutsideTemperature(data.temperature);
        Dashboard::instance().updateOutsideHumidity(data.humidity);
        Dashboard::instance().updateOutsideWindSpeed(data.windSpeed);
        Dashboard::instance().updateOutsideFeelsLike(data.feelsLike);
        Dashboard::instance().updateOutsideDailyHigh(data.tempMax);
        Dashboard::instance().updateOutsideDailyLow(data.tempMin);
        Dashboard::instance().updateOutsidePressure(data.pressure);
        ui->currentWeatherBody.setDescription(data.description);
        ui->currentWeatherBody.setFeelsLikeTemp(data.feelsLike);
        ui->currentWeatherBody.setIcon(data.icon);
        ui->currentWeatherBody.setCurrentParams(data);
        ui->header.setCity(data.city);
        ui->header.setCountry(data.country);
        ui->header.setCurrentTime(data.timestamp);
    }

    void setForecast(Weather::Data* forecast)
    {
        ui->todayForecastTable.setForecast(forecast);
        ui->forecastTable.set(forecast);
        Dashboard::instance().updateOutsidePrecipitation(
            Weather::instance().getNext24hPrecipitation());
    }

    void setSSID(char* newSSID)
    {
        ui->header.setSSID(newSSID);
    }

    void updateRSSI(int8_t rssi)
    {
        ui->header.updateRSSI(rssi);
    }
#ifdef COMMON_DEMO_APP
    void updateWeatherValues()
    {
        if (weatherInfo->timestamp == 0)
            return;
        if (CurrentTime::instance().isTimeSet())
        {
            time(&curTimestamp);
            weatherInfo->timestamp = curTimestamp + weatherInfo->timestampOffset;
            BM8563::instance().setUnixTimeStamp(weatherInfo->timestamp);
            //  TimeStamp::instance().is_sync_current_time = 1;
        }
        CurrentTime::instance().setTimezoneOffset(weatherInfo->timestampOffset);
        ESP_LOGD("WeatherScreen", "weatherInfo->timestampOffset: time=%lu",
                 weatherInfo->timestampOffset);
        TimeStamp::instance().is_sync_current_time = 1;
        Dashboard::instance().updateTimeLabel(weatherInfo->timestamp, weatherInfo->timestampOffset);
        WeatherScreen::instance().setCurrentWeather(*weatherInfo);
        WifiScreen::instance().setLocation(*weatherInfo);
        WeatherScreen::instance().setForecast(forecast_);
        Brightness::instance().update(CurrentTime::instance().isTimeSet() ? true : false);
    }

    bool updateWeather()
    {
        if (!Weather::instance().getCurrentWeather(weatherInfo) ||
            !Weather::instance().getForecast(forecast_))
        {
            return false;
        }

        updateWeatherValues();
        return true;
    }
#endif
};

