#pragma once

#include "adapters/lvgl/FlexContainer.h"
#include "entities/Units.h"
#include "entities/Weather.h"
#include "entities/ui/sensors_settings/SensorSettings.h"
#include "entities/ui/weather_screen/elements/ParameterLabel.h"
#include "entities/ui/weather_screen/elements/SmallTimestampLabel.h"
#include "entities/ui/weather_screen/elements/WeatherIcon.h"
#include <esp_log.h>

class TodayForecastTable
{
    static constexpr char*   Tag                        = "TodayTable";
    static constexpr uint8_t TodayWeatherForecastsCount = 4;
    SensorSettings*          sensor_settings_;
    FlexContainer            mainContainer;
    FlexContainer            forecastContainer[TodayWeatherForecastsCount];
#ifdef COMMON_DEMO_APP
    FlexContainer parametersContainer[TodayWeatherForecastsCount];
#endif
    WeatherIcon         forecastIcon[TodayWeatherForecastsCount];
    SmallTimestampLabel timestampForecastLabel[TodayWeatherForecastsCount];
    ParameterLabel      tempSmallLabel[TodayWeatherForecastsCount];

public:
    void create(SensorSettings* sensor_settings, lv_obj_t* parent)
    {
        sensor_settings_ = sensor_settings;
        // mainContainer (row)
#ifdef COMMON_DEMO_APP
        mainContainer.create(parent, lv_pct(100), lv_pct(16), LV_FLEX_FLOW_ROW);
#else
        mainContainer.create(parent, lv_pct(100), lv_pct(22), LV_FLEX_FLOW_ROW);
#endif
        mainContainer.align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START);
        mainContainer.padding(10, 0);
        lv_obj_clear_flag(mainContainer.get(), LV_OBJ_FLAG_SCROLLABLE);
        {
            for (uint8_t i = 0; i < TodayWeatherForecastsCount; i++)
            {
                // forecastContainer[i] (column)
                forecastContainer[i].create(mainContainer.get(), lv_pct(25), LV_SIZE_CONTENT,
                                            LV_FLEX_FLOW_COLUMN);
                forecastContainer[i].align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                                           LV_FLEX_ALIGN_CENTER);
                {
                    /// timestampLabel
                    timestampForecastLabel[i].create(forecastContainer[i].get(),
                                                     &lv_font_montserrat_16);
                    lv_obj_t* dataParent = forecastContainer[i].get();
#ifdef COMMON_DEMO_APP
                    parametersContainer[i].create(forecastContainer[i].get(), lv_pct(100),
                                                  LV_SIZE_CONTENT, LV_FLEX_FLOW_ROW);
                    dataParent = parametersContainer[i].get();
                    parametersContainer[i].align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                                                 LV_FLEX_ALIGN_CENTER);
#endif

                    /// icon
                    forecastIcon[i].create(dataParent);
                    forecastIcon[i].scale(0.5f);

                    /// smallTemperatureLabel
                    tempSmallLabel[i].create(dataParent, &lv_font_montserrat_16);
                    tempSmallLabel[i].setPostfix(TEMPERATURE_POSTFIX);
                }
            }
        }
    }

    void setForecast(Weather::Data* data)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            timestampForecastLabel[i].set(data[i].timestamp);
            tempSmallLabel[i].setPostfix(unit_names.at(sensor_settings_->temperature));
            tempSmallLabel[i].setParam(
                convertValueToUnit(sensor_settings_->temperature, data[i].temperature), true);
            forecastIcon[i].set(data[i].icon);
        }
    }
};