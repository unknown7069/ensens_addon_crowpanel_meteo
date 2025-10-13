#include "Dashboard.h"

#include "entities/ui/weather_screen/WeatherScreen.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <limits>

lv_obj_t* Dashboard::create(SensorSettings* sensor_settings, lv_obj_t* parent)
{
    lock();
    sensor_settings_ = sensor_settings;

    tv_ = tabview_create(parent, 25);
    create_main_elements(tv_->tab_1);
    WeatherScreen::instance().create(sensor_settings_, tv_);
    WifiScreen::instance().create(sensor_settings_, tv_->tab_settings);
    WifiScreen::instance().loadSettings();

    setupBottomPlotSources();

    unlock();
    updateBottomPlot();
    return cont_;
}

void Dashboard::setupBottomPlotSources()
{
    if (tv_ == nullptr)
        return;

    static const IndoorMetricPlot kTemperatureMetric = IndoorMetricPlot::Temperature;
    static const IndoorMetricPlot kHumidityMetric    = IndoorMetricPlot::Humidity;
    static const IndoorMetricPlot kPressureMetric    = IndoorMetricPlot::Pressure;
    static const IndoorMetricPlot kCo2Metric         = IndoorMetricPlot::CO2;
    static const IndoorMetricPlot kVocMetric         = IndoorMetricPlot::VOC;
    static const IndoorMetricPlot kIaqMetric         = IndoorMetricPlot::IAQ;

    struct Binding {
        lv_obj_t**                    widget;
        const IndoorMetricPlot*       metric;
    };

    Binding bindings[] = {
        { &tv_->temp_inside_label, &kTemperatureMetric },
        { &tv_->humidity_inside_label, &kHumidityMetric },
        { &tv_->pressure_inside_label, &kPressureMetric },
        { &tv_->co2_label, &kCo2Metric },
        { &tv_->voc_label, &kVocMetric },
        { &tv_->iaq_label, &kIaqMetric },
    };

    for (const auto& binding : bindings)
    {
        if (binding.widget == nullptr || *binding.widget == nullptr)
            continue;
        lv_obj_t* obj = *binding.widget;
        lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(obj, bottom_plot_source_event_cb, LV_EVENT_CLICKED,
                            const_cast<IndoorMetricPlot*>(binding.metric));
    }
}

void Dashboard::selectBottomPlotMetric(IndoorMetricPlot metric)
{
    updateBottomPlotInternal(metric);
}

void Dashboard::updateBottomPlot()
{
    updateBottomPlotInternal(bottom_plot_metric_);
}

void Dashboard::updateBottomPlotInternal(IndoorMetricPlot metric)
{
    lock();
    bottom_plot_metric_ = metric;

    if (tv_ == nullptr || sensor_settings_ == nullptr || tv_->bottom_plot_chart == nullptr ||
        tv_->bottom_plot_series == nullptr)
    {
        unlock();
        return;
    }

    const char* metric_label = "";
    const char* unit_label   = "";
    int32_t     default_min  = 0;
    int32_t     default_max  = 100;

    EnvironmentalSensor::Parameters parameter = parameterFromMetric(metric);

    switch (metric)
    {
    case IndoorMetricPlot::Temperature:
        metric_label = "Indoor Temperature";
        unit_label   = unit_names.at(sensor_settings_->temperature);
        default_min  = -10;
        default_max  = 40;
        break;
    case IndoorMetricPlot::Humidity:
        metric_label = "Indoor Humidity";
        unit_label   = "%";
        default_min  = 0;
        default_max  = 100;
        break;
    case IndoorMetricPlot::Pressure:
        metric_label = "Indoor Pressure";
        unit_label   = unit_names.at(sensor_settings_->pressure);
        default_min  = 900;
        default_max  = 1100;
        break;
    case IndoorMetricPlot::CO2:
        metric_label = "Indoor CO2";
        unit_label   = "ppm";
        default_min  = 0;
        default_max  = 2000;
        break;
    case IndoorMetricPlot::VOC:
        metric_label = "Indoor VOC";
        unit_label   = "ppb";
        default_min  = 0;
        default_max  = 2000;
        break;
    case IndoorMetricPlot::IAQ:
        metric_label = "Indoor IAQ";
        unit_label   = "index";
        default_min  = 0;
        default_max  = 500;
        break;
    default:
        unlock();
        return;
    }

    std::array<EnvironmentalSensor::DataSample<float>, DailyMetricHistory::SlotsPerDay> samples{};
    std::array<bool, DailyMetricHistory::SlotsPerDay>                                    has_value{};
    bool series_available =
        Aggregator::instance().getIndoorMetricSeries(sensor_settings_->sensor_name, parameter,
                                                     samples, has_value);

    const uint16_t desired_points = static_cast<uint16_t>(DailyMetricHistory::SlotsPerDay);
    if (lv_chart_get_point_count(tv_->bottom_plot_chart) != desired_points)
    {
        lv_chart_set_point_count(tv_->bottom_plot_chart, desired_points);
    }

    bool  has_any_value = false;
    float min_value     = std::numeric_limits<float>::max();
    float max_value     = std::numeric_limits<float>::lowest();

    for (size_t i = 0; i < DailyMetricHistory::SlotsPerDay; ++i)
    {
        lv_coord_t chart_value = LV_CHART_POINT_NONE;
        if (series_available && has_value[i])
        {
            float value = samples[i].value;
            min_value   = std::min(min_value, value);
            max_value   = std::max(max_value, value);
            chart_value = static_cast<lv_coord_t>(std::lround(value));
            has_any_value = true;
        }

        lv_chart_set_value_by_id(tv_->bottom_plot_chart, tv_->bottom_plot_series,
                                 static_cast<uint16_t>(i), chart_value);
    }

    int32_t axis_min = default_min;
    int32_t axis_max = default_max;

    if (has_any_value)
    {
        float range   = max_value - min_value;
        float padding = range * 0.1f;
        if (padding < 1.0f)
            padding = 1.0f;

        const float padded_min = min_value - padding;
        const float padded_max = max_value + padding;

        axis_min = static_cast<int32_t>(std::floor(padded_min));
        axis_max = static_cast<int32_t>(std::ceil(padded_max));
        if (axis_min == axis_max)
            axis_max = axis_min + 1;
    }

    lv_chart_set_range(tv_->bottom_plot_chart, LV_CHART_AXIS_PRIMARY_Y, axis_min, axis_max);

    char title_buffer[64];
    std::snprintf(title_buffer, sizeof(title_buffer), "%s (%s)", metric_label, unit_label);
    lv_label_set_text(tv_->bottom_plot_title, title_buffer);

    if (tv_->bottom_plot_cursor)
    {
        uint32_t slot_minutes = DailyMetricHistory::SlotDurationSecond / 60U;
        if (slot_minutes == 0)
            slot_minutes = 1;

        uint32_t timestamp = current_timestamp_;
        if (timestamp == 0)
        {
            time_t now = time(nullptr);
            timestamp  = static_cast<uint32_t>(now);
        }

        time_t    raw_time = static_cast<time_t>(timestamp);
        struct tm local_time {};
#if defined(_MSC_VER)
        if (localtime_s(&local_time, &raw_time) != 0)
        {
            std::memset(&local_time, 0, sizeof(local_time));
        }
#else
        if (localtime_r(&raw_time, &local_time) == nullptr)
        {
            std::memset(&local_time, 0, sizeof(local_time));
        }
#endif

        uint32_t minutes_since_midnight = static_cast<uint32_t>(local_time.tm_hour) * 60U +
                                          static_cast<uint32_t>(local_time.tm_min);
        uint16_t slot_index =
            static_cast<uint16_t>(minutes_since_midnight / slot_minutes);
        if (slot_index >= DailyMetricHistory::SlotsPerDay)
            slot_index = DailyMetricHistory::SlotsPerDay - 1;

        lv_point_t cursor_pos = { 0, 0 };
        lv_chart_get_point_pos_by_id(tv_->bottom_plot_chart, tv_->bottom_plot_series, slot_index,
                                     &cursor_pos);
        cursor_pos.y = 0;
        lv_chart_set_cursor_pos(tv_->bottom_plot_chart, tv_->bottom_plot_cursor, &cursor_pos);
    }

    lv_chart_refresh(tv_->bottom_plot_chart);
    unlock();
}

void Dashboard::bottom_plot_source_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;

    const auto* metric_ptr =
        static_cast<const IndoorMetricPlot*>(lv_event_get_user_data(e));
    if (metric_ptr == nullptr)
        return;

    Dashboard::instance().selectBottomPlotMetric(*metric_ptr);
}

bool Dashboard::mapParameterToMetric(EnvironmentalSensor::Parameters param,
                                     IndoorMetricPlot& metric) const
{
    switch (param)
    {
    case EnvironmentalSensor::Temperature:
        metric = IndoorMetricPlot::Temperature;
        return true;
    case EnvironmentalSensor::Humidity:
        metric = IndoorMetricPlot::Humidity;
        return true;
    case EnvironmentalSensor::Pressure:
        metric = IndoorMetricPlot::Pressure;
        return true;
    case EnvironmentalSensor::CO2:
        metric = IndoorMetricPlot::CO2;
        return true;
    case EnvironmentalSensor::VOC:
        metric = IndoorMetricPlot::VOC;
        return true;
    case EnvironmentalSensor::IAQ:
        metric = IndoorMetricPlot::IAQ;
        return true;
    default:
        return false;
    }
}

EnvironmentalSensor::Parameters Dashboard::parameterFromMetric(IndoorMetricPlot metric) const
{
    switch (metric)
    {
    case IndoorMetricPlot::Temperature:
        return EnvironmentalSensor::Temperature;
    case IndoorMetricPlot::Humidity:
        return EnvironmentalSensor::Humidity;
    case IndoorMetricPlot::Pressure:
        return EnvironmentalSensor::Pressure;
    case IndoorMetricPlot::CO2:
        return EnvironmentalSensor::CO2;
    case IndoorMetricPlot::VOC:
        return EnvironmentalSensor::VOC;
    case IndoorMetricPlot::IAQ:
        return EnvironmentalSensor::IAQ;
    default:
        return EnvironmentalSensor::Temperature;
    }
}

void Dashboard::handleIndoorMetricUpdate(const std::string& dev_name,
                                         EnvironmentalSensor::Parameters param)
{
    if (sensor_settings_ == nullptr)
        return;

    if (dev_name != sensor_settings_->sensor_name)
        return;

    IndoorMetricPlot metric;
    if (!mapParameterToMetric(param, metric))
        return;

    if (metric != bottom_plot_metric_)
        return;

    updateBottomPlot();
}

void Dashboard::updateUnitNames()
{
    lock();
    const char* pressure_unit_name = unit_names.at(sensor_settings_->pressure);
    lv_label_set_text(pressure_box_->title, pressure_unit_name);
    const char* temperature_unit_name = unit_names.at(sensor_settings_->temperature);
    lv_label_set_text(temp_box_->value->unit_part, temperature_unit_name);
    lv_label_set_text(humi_dew_point_box_->value_dew->unit_part, temperature_unit_name);
    unlock();
}

void Dashboard::updateSettings(const std::string& old_dev_name)
{
    lock();
    updateUnitNames();

    EnvironmentalSensor::DataSample<float> temperature;
    Aggregator::instance().getTemperatureData(sensor_settings_->sensor_name, temperature);

    EnvironmentalSensor::DataSample<float> pressure;
    Aggregator::instance().getPressureData(sensor_settings_->sensor_name, pressure);

    updatePressureBox(sensor_settings_->pressure);
    EnvironmentalSensor::DataSample<float> old_pressure;
    Aggregator::instance().getPressureData(old_dev_name, old_pressure);
    updatePressurePlotData(old_pressure.unit);

    convertToDefault(temperature);
    convertToUnit(sensor_settings_->temperature, temperature);
    updateTemperature(sensor_settings_->sensor_name, temperature.value);

    convertToDefault(pressure);
    convertToUnit(sensor_settings_->pressure, pressure);
    updatePressure(sensor_settings_->sensor_name, pressure.value);
    Aggregator::instance().setPressureData(sensor_settings_->sensor_name, pressure);

    updateDewPoint(sensor_settings_->sensor_name);

    WeatherScreen::instance().updateWeatherValues();

    unlock();
    updateBottomPlot();
}

std::string Dashboard::getZodiacSign(uint32_t timestamp)
{
    if (timestamp != 0)
    {
        time_t     timestampStruct = timestamp;
        struct tm* timeInfo        = localtime(&timestampStruct);

        int month = timeInfo->tm_mon + 1; // tm_mon is 0-11
        int day   = timeInfo->tm_mday;

        // Determine zodiac sign
        if ((month == 1 && day >= 20) || (month == 2 && day <= 18))
            return "Aquarius";
        if ((month == 2 && day >= 19) || (month == 3 && day <= 20))
            return "Pisces";
        if ((month == 3 && day >= 21) || (month == 4 && day <= 19))
            return "Aries";
        if ((month == 4 && day >= 20) || (month == 5 && day <= 20))
            return "Taurus";
        if ((month == 5 && day >= 21) || (month == 6 && day <= 20))
            return "Gemini";
        if ((month == 6 && day >= 21) || (month == 7 && day <= 22))
            return "Cancer";
        if ((month == 7 && day >= 23) || (month == 8 && day <= 22))
            return "Leo";
        if ((month == 8 && day >= 23) || (month == 9 && day <= 22))
            return "Virgo";
        if ((month == 9 && day >= 23) || (month == 10 && day <= 22))
            return "Libra";
        if ((month == 10 && day >= 23) || (month == 11 && day <= 21))
            return "Scorpio";
        if ((month == 11 && day >= 22) || (month == 12 && day <= 21))
            return "Sagittarius";
        if ((month == 12 && day >= 22) || (month == 1 && day <= 19))
            return "Capricorn";
        return "Unknown"; // Should never reach here
    } else
        return "";

    return "Unknown";
}

int Dashboard::getDayOfWeek(uint32_t timestamp)
{
    if (timestamp != 0)
    {
        time_t     timestampStruct = timestamp;
        struct tm* timeInfo        = localtime(&timestampStruct);

        return timeInfo->tm_wday; // days since Sunday - [0, 6]
    } else
        return 0;

    return 0;
}
