#pragma once

#include "esp_log.h"
#include "lvgl.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "Aggregator.h"
#include "entities/EnvironmentalSensorData.h"
#include "entities/Units.h"
#include "entities/ui/Dashboard/DashboardWidgets.h"
#include "entities/ui/sensors_settings/SensorSettings.h"

#include "entities/ui/wifi_screen/WifiScreen.h"

#include "adapters/lvgl/lvgl_port_v8.h"
#include "settings.h"

#include <cstdint>
#include <initializer_list>
#include <string>
#include <utility>

class Dashboard
{
public:
    enum class IndoorMetricPlot : uint8_t { Temperature, Humidity, Pressure, CO2, VOC, IAQ };

private:
    static constexpr char TAG[] = "Dashboard";

    DashboardWidgets* widgets_;
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

    // Renders "placeholder" when value is NaN, otherwise formats it as
    // "<number><suffix>" using a printf-style float format such as "%+.1f%s".
    // Caller must hold the LVGL lock.
    static void updateNumericLabel(lv_obj_t* label, float value, const char* placeholder,
                                   const char* number_format, const char* suffix);

    // Takes the LVGL lock and reports whether the label may be updated for the
    // currently selected sensor.
    bool beginSelectedLabelUpdate(const std::string& dev_name, lv_obj_t* label);

public:
    static Dashboard& instance()
    {
        static Dashboard instance;
        return instance;
    }

    DashboardWidgets* getTabView()
    {
        return widgets_;
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

    void updateTemperature(const std::string& dev_name, float value);
    void updateHumidity(const std::string& dev_name, float value);
    void updatePressure(const std::string& dev_name, float value);
    void updateCO2(const std::string& dev_name, uint16_t value);
    void updateVOC(const std::string& dev_name, uint16_t value);
    void updateIAQ(const std::string& dev_name, uint16_t value);

    void updateOutsideTemperature(float value);
    void updateOutsideFeelsLike(float value);
    void updateOutsideDailyHigh(float value);
    void updateOutsideDailyLow(float value);
    void updateOutsidePressure(float value);
    void updateOutsideWindSpeed(float value);
    void updateOutsideHumidity(float value);
    void updateOutsidePrecipitation(float value);

    void updateTimeLabel(uint32_t timestamp, int32_t timestampOffset);

    static lv_color_t getQualityColor(uint16_t value, std::initializer_list<std::pair<uint16_t, lv_palette_t>> ranges)
    {
        for (const auto& entry : ranges)
        {
            if (value <= entry.first)
                return lv_palette_main(entry.second);
        }
        return lv_palette_main(LV_PALETTE_RED);
    }

    void updateSettings(const std::string&);

    void updateSensorData(const std::string& old_dev_name, const std::string& dev_name);
};
