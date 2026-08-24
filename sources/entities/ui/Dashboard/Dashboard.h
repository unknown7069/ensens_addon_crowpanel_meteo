#pragma once

#include "esp_log.h"
#include "esp_log_level.h"
#include "lvgl.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "Aggregator.h"
#include "entities/EnvironmentalSensorData.h"
#include "entities/ui/components/common.h"
#include "entities/Units.h"
#include "entities/ui/sensors_settings/SensorSettings.h"

#include "entities/ui/wifi_screen/WifiScreen.h"

#include "adapters/lvgl/lvgl_port_v8.h"
#include "settings.h"
#include <cmath>

#include <iterator>
#include <sstream>
#include <initializer_list>
#include <utility>

class Dashboard
{
    static constexpr char TAG[] = "Dashboard";
    enum class IndoorMetricPlot : uint8_t { Temperature, Humidity, Pressure, CO2, VOC, IAQ };

    tabview_t*       tv_;
    SensorSettings*  sensor_settings_;
    IndoorMetricPlot bottom_plot_metric_ = IndoorMetricPlot::Temperature;
    uint32_t         current_timestamp_  = 0;

    void setupBottomPlotSources();
    void selectBottomPlotMetric(IndoorMetricPlot metric);
    void updateBottomPlotInternal(IndoorMetricPlot metric);
    void updateBottomPlot();
    static void bottom_plot_source_event_cb(lv_event_t* e);
    bool mapParameterToMetric(EnvironmentalSensor::Parameters param, IndoorMetricPlot& metric) const;
    EnvironmentalSensor::Parameters parameterFromMetric(IndoorMetricPlot metric) const;

    Dashboard()                            = default;
    Dashboard(const Dashboard&)            = delete;
    Dashboard& operator=(const Dashboard&) = delete;

    void lock()
    {
        lvgl_port_lock(-1);
    }

    void unlock()
    {
        lvgl_port_unlock();
    }

public:
    static Dashboard& instance()
    {
        static Dashboard instance;
        return instance;
    }

    tabview_t* getTabView()
    {
        return tv_;
    }

    uint32_t getCurrentTimestamp()
    {
        lock();
        uint32_t ts = current_timestamp_;
        unlock();
        return ts;
    }

    void handleIndoorMetricUpdate(const std::string& dev_name, EnvironmentalSensor::Parameters param);

    lv_obj_t* create(SensorSettings* sensor_settings, lv_obj_t* parent);

    void updateTemperature(const std::string& dev_name, const float value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        if (tv_ && tv_->temp_inside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->temp_inside_label, "--");
            } else
            {
                const char* unit = unit_names.at(sensor_settings_->temperature);
                lv_label_set_text_fmt(tv_->temp_inside_label, "%+.1f%s", value, unit);
            }
        }
        unlock();
    }

    void updateOutsideTemperature(float value)
    {
        lock();
        if (tv_ && tv_->temp_outside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->temp_outside_label, "--");
            } else
            {
                float converted = convertValueToUnit(sensor_settings_->temperature, value);
                const char* unit = unit_names.at(sensor_settings_->temperature);
                lv_label_set_text_fmt(tv_->temp_outside_label, "%+.1f%s", converted, unit);
            }
        }
        unlock();
    }

    void updateOutsidePressure(float value)
    {
        lock();
        if (tv_ && tv_->pressure_outside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->pressure_outside_label, "--");
            } else
            {
                float converted = convertValueToDefault(UnitType::hPa, value);
                converted = convertValueToUnit(sensor_settings_->pressure, converted);
                const char* unit = unit_names.at(sensor_settings_->pressure);
                lv_label_set_text_fmt(tv_->pressure_outside_label, "%.0f %s", converted, unit);
            }
        }
        unlock();
    }

    void updateOutsideWindSpeed(float value)
    {
        lock();
        if (tv_ && tv_->wind_speed_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->wind_speed_label, "--");
            } else
            {
                lv_label_set_text_fmt(tv_->wind_speed_label, "%.1f m/s", value);
            }
        }
        unlock();
    }
    void updateOutsideFeelsLike(float value)
    {
        lock();
        if (tv_ && tv_->feels_like_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->feels_like_label, "--");
            } else
            {
                float converted = convertValueToUnit(sensor_settings_->temperature, value);
                const char* unit = unit_names.at(sensor_settings_->temperature);
                lv_label_set_text_fmt(tv_->feels_like_label, "%+.1f%s", converted, unit);
            }
        }
        unlock();
    }

    void updateOutsideDailyHigh(float value)
    {
        lock();
        if (tv_ && tv_->daily_high_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->daily_high_label, "--");
            } else
            {
                float converted = convertValueToUnit(sensor_settings_->temperature, value);
                const char* unit = unit_names.at(sensor_settings_->temperature);
                lv_label_set_text_fmt(tv_->daily_high_label, "%.1f%s", converted, unit);
            }
        }
        unlock();
    }

    void updateOutsideDailyLow(float value)
    {
        lock();
        if (tv_ && tv_->daily_low_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->daily_low_label, "--");
            } else
            {
                float converted = convertValueToUnit(sensor_settings_->temperature, value);
                const char* unit = unit_names.at(sensor_settings_->temperature);
                lv_label_set_text_fmt(tv_->daily_low_label, "%.1f%s", converted, unit);
            }
        }
        unlock();
    }

    void updateOutsideHumidity(float value)
    {
        lock();
        if (tv_ && tv_->humidity_outside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->humidity_outside_label, "--%");
            } else
            {
                lv_label_set_text_fmt(tv_->humidity_outside_label, "%.0f%%", value);
            }
        }
        unlock();
    }

    void updateOutsidePrecipitation(float value)
    {
        lock();
        if (tv_ && tv_->precipitation_outside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->precipitation_outside_label, "-- mm");
            } else
            {
                lv_label_set_text_fmt(tv_->precipitation_outside_label, "%.1f mm", value);
            }
        }
        unlock();
    }

    void updateHumidity(const std::string& dev_name, const float value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        if (tv_ && tv_->humidity_inside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->humidity_inside_label, "--%");
            } else
            {
                lv_label_set_text_fmt(tv_->humidity_inside_label, "%.0f%%", value);
            }
        }
        unlock();
    }

    void updatePressure(const std::string& dev_name, const float value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        if (tv_ && tv_->pressure_inside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->pressure_inside_label, "--");
            } else
            {
                const char* unit = unit_names.at(sensor_settings_->pressure);
                lv_label_set_text_fmt(tv_->pressure_inside_label, "%.0f %s", value, unit);
            }
        }
        unlock();
    }

    void updateCO2(const std::string& dev_name, const uint16_t value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        if (tv_ && tv_->co2_label)
        {
            auto color = get_quality_color(value, { { 800, LV_PALETTE_GREEN }, { 1200, LV_PALETTE_YELLOW }, { 1600, LV_PALETTE_ORANGE } });
            lv_obj_set_style_text_color(tv_->co2_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_fmt(tv_->co2_label, "%u ppm", value);
        }
        unlock();
    }

    void updateVOC(const std::string& dev_name, const uint16_t value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        if (tv_ && tv_->voc_label)
        {
            auto color = get_quality_color(value, { { 200, LV_PALETTE_GREEN }, { 400, LV_PALETTE_YELLOW }, { 1000, LV_PALETTE_ORANGE } });
            lv_obj_set_style_text_color(tv_->voc_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_fmt(tv_->voc_label, "%u ppb", value);
        }
        unlock();
    }

    void updateIAQ(const std::string& dev_name, const uint16_t value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        if (tv_ && tv_->iaq_label)
        {
            auto color = get_quality_color(value, { { 50, LV_PALETTE_GREEN }, { 100, LV_PALETTE_YELLOW }, { 150, LV_PALETTE_ORANGE } });
            lv_obj_set_style_text_color(tv_->iaq_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_fmt(tv_->iaq_label, "%u", value);
        }
        unlock();
    }

    void updateTimeLabel(uint32_t timestamp, int32_t timestampOffset)
    {
        lock();

        current_timestamp_ = timestamp;
        time_t localTimestamp = static_cast<time_t>(timestamp) + timestampOffset;

        if (tv_)
        {
            if (localTimestamp != 0)
            {
                time_t   timestampStruct = localTimestamp;
                struct tm* timeInfo      = localtime(&timestampStruct);

                if (tv_->label)
                {
                    char time_text[6] = { 0 };
                    strftime(time_text, sizeof(time_text), "%H:%M", timeInfo);
                    lv_label_set_text(tv_->label, time_text);
                }

                if (tv_->date_label)
                {
                    char date_text[24] = { 0 };
                    strftime(date_text, sizeof(date_text), "%a %d %b", timeInfo);
                    lv_label_set_text(tv_->date_label, date_text);
                }
            } else
            {
                if (tv_->label)
                    lv_label_set_text(tv_->label, "");
                if (tv_->date_label)
                    lv_label_set_text(tv_->date_label, "");
            }
        }

        unlock();
    }

    static lv_color_t get_quality_color(uint16_t value, std::initializer_list<std::pair<uint16_t, lv_palette_t>> ranges)
    {
        for (const auto& entry : ranges)
        {
            if (value <= entry.first)
                return lv_palette_main(entry.second);
        }
        return lv_palette_main(LV_PALETTE_RED);
    }

    void updateSettings(const std::string& old_dev_name);

    void updateSensorData(const std::string& old_dev_name, const std::string& dev_name)
    {
        EnvironmentalSensor::DataSample<float> temperature;
        EnvironmentalSensor::DataSample<float> humidity;
        EnvironmentalSensor::DataSample<float> pressure;
        EnvironmentalSensor::DataSample<float> co2;
        EnvironmentalSensor::DataSample<float> voc;
        EnvironmentalSensor::DataSample<float> iaq;

        Aggregator::instance().getTemperatureData(dev_name, temperature);
        Aggregator::instance().getHumidityData(dev_name, humidity);
        Aggregator::instance().getPressureData(dev_name, pressure);
        Aggregator::instance().getCO2Data(dev_name, co2);
        Aggregator::instance().getVOCData(dev_name, voc);
        Aggregator::instance().getIAQData(dev_name, iaq);

        updateTemperature(dev_name, temperature.value);
        updateHumidity(dev_name, humidity.value);
        updatePressure(dev_name, pressure.value);
        updateCO2(dev_name, co2.value);
        updateVOC(dev_name, voc.value);
        updateIAQ(dev_name, iaq.value);

        updateSettings(old_dev_name);
    }
};





