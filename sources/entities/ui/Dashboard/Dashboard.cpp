#include "Dashboard.h"

#include "usecases/WeatherUpdate.h"
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

    widgets_ = dashboard_view_create(parent, 25);
    WifiScreen::instance().create(sensor_settings_, widgets_->tab_settings);
    WifiScreen::instance().loadSettings();

    setupBottomPlotSources();

    unlock();
    updateBottomPlot();
    return widgets_->cont;
}

void Dashboard::setupBottomPlotSources()
{
    if (widgets_ == nullptr)
        return;

    static const IndoorMetricPlot kTemperatureMetric = IndoorMetricPlot::Temperature;
    static const IndoorMetricPlot kHumidityMetric    = IndoorMetricPlot::Humidity;
    static const IndoorMetricPlot kPressureMetric    = IndoorMetricPlot::Pressure;
    static const IndoorMetricPlot kCo2Metric         = IndoorMetricPlot::CO2;
    static const IndoorMetricPlot kVocMetric         = IndoorMetricPlot::VOC;
    static const IndoorMetricPlot kIaqMetric         = IndoorMetricPlot::IAQ;

    struct Binding {
        lv_obj_t**              widget;
        const IndoorMetricPlot* metric;
    };

    Binding bindings[] = {
        { &widgets_->temp_inside_label, &kTemperatureMetric },
        { &widgets_->humidity_inside_label, &kHumidityMetric },
        { &widgets_->pressure_inside_label, &kPressureMetric },
        { &widgets_->co2_label, &kCo2Metric },
        { &widgets_->voc_label, &kVocMetric },
        { &widgets_->iaq_label, &kIaqMetric },
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

    if (widgets_ == nullptr || sensor_settings_ == nullptr || widgets_->bottom_plot_chart == nullptr ||
        widgets_->bottom_plot_series == nullptr)
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
    if (lv_chart_get_point_count(widgets_->bottom_plot_chart) != desired_points)
    {
        lv_chart_set_point_count(widgets_->bottom_plot_chart, desired_points);
    }

    bool  has_any_value = false;
    float min_value     = std::numeric_limits<float>::max();
    float max_value     = std::numeric_limits<float>::lowest();

    for (size_t i = 0; i < DailyMetricHistory::SlotsPerDay; ++i)
    {
        lv_coord_t chart_value = LV_CHART_POINT_NONE;
        if (series_available && has_value[i])
        {
            float value   = samples[i].value;
            min_value     = std::min(min_value, value);
            max_value     = std::max(max_value, value);
            chart_value   = static_cast<lv_coord_t>(std::lround(value));
            has_any_value = true;
        }

        lv_chart_set_value_by_id(widgets_->bottom_plot_chart, widgets_->bottom_plot_series,
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

    lv_chart_set_range(widgets_->bottom_plot_chart, LV_CHART_AXIS_PRIMARY_Y, axis_min, axis_max);

    char title_buffer[64];
    std::snprintf(title_buffer, sizeof(title_buffer), "%s (%s)", metric_label, unit_label);
    lv_label_set_text(widgets_->bottom_plot_title, title_buffer);

    if (widgets_->bottom_plot_cursor)
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
        uint16_t slot_index = static_cast<uint16_t>(minutes_since_midnight / slot_minutes);
        if (slot_index >= DailyMetricHistory::SlotsPerDay)
            slot_index = DailyMetricHistory::SlotsPerDay - 1;

        lv_point_t cursor_pos = { 0, 0 };
        lv_chart_get_point_pos_by_id(widgets_->bottom_plot_chart, widgets_->bottom_plot_series, slot_index,
                                     &cursor_pos);
        cursor_pos.y = 0;
        lv_chart_set_cursor_pos(widgets_->bottom_plot_chart, widgets_->bottom_plot_cursor, &cursor_pos);
    }

    lv_chart_refresh(widgets_->bottom_plot_chart);
    unlock();
}

void Dashboard::bottom_plot_source_event_cb(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;

    const auto* metric_ptr = static_cast<const IndoorMetricPlot*>(lv_event_get_user_data(e));
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

bool Dashboard::beginSelectedLabelUpdate(const std::string& dev_name, lv_obj_t* label)
{
    lock();
    if (label == nullptr || sensor_settings_ == nullptr ||
        dev_name != sensor_settings_->sensor_name)
    {
        unlock();
        return false;
    }
    return true;
}

void Dashboard::updateNumericLabel(lv_obj_t* label, float value, const char* placeholder,
                                   const char* number_format, const char* suffix)
{
    if (label == nullptr)
        return;

    if (std::isnan(value))
    {
        lv_label_set_text(label, placeholder);
        return;
    }

    char text[32] = { 0 };
    snprintf(text, sizeof(text), number_format, value, suffix);
    lv_label_set_text(label, text);
}

void Dashboard::updateTemperature(const std::string& dev_name, float value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->temp_inside_label))
        return;

    updateNumericLabel(widgets_->temp_inside_label, value, "--", "%+.1f%s",
                       unit_names.at(sensor_settings_->temperature));
    unlock();
}

void Dashboard::updateHumidity(const std::string& dev_name, float value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->humidity_inside_label))
        return;

    updateNumericLabel(widgets_->humidity_inside_label, value, "--%", "%.0f%s", "%");
    unlock();
}

void Dashboard::updatePressure(const std::string& dev_name, float value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->pressure_inside_label))
        return;

    updateNumericLabel(widgets_->pressure_inside_label, value, "--", "%.0f %s",
                       unit_names.at(sensor_settings_->pressure));
    unlock();
}

namespace
{
// Air quality color bands: a value is colored by the first threshold it does
// not exceed; anything above the last band renders red.
constexpr std::initializer_list<std::pair<uint16_t, lv_palette_t>> kCo2QualityRangesPpm = {
    { 800, LV_PALETTE_GREEN }, { 1200, LV_PALETTE_YELLOW }, { 1600, LV_PALETTE_ORANGE }
};
constexpr std::initializer_list<std::pair<uint16_t, lv_palette_t>> kVocQualityRangesPpb = {
    { 200, LV_PALETTE_GREEN }, { 400, LV_PALETTE_YELLOW }, { 1000, LV_PALETTE_ORANGE }
};
constexpr std::initializer_list<std::pair<uint16_t, lv_palette_t>> kIaqQualityRangesIndex = {
    { 50, LV_PALETTE_GREEN }, { 100, LV_PALETTE_YELLOW }, { 150, LV_PALETTE_ORANGE }
};
} // namespace

void Dashboard::updateCO2(const std::string& dev_name, uint16_t value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->co2_label))
        return;

    auto color = getQualityColor(value, kCo2QualityRangesPpm);
    lv_obj_set_style_text_color(widgets_->co2_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(widgets_->co2_label, "%u ppm", value);
    unlock();
}

void Dashboard::updateVOC(const std::string& dev_name, uint16_t value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->voc_label))
        return;

    auto color = getQualityColor(value, kVocQualityRangesPpb);
    lv_obj_set_style_text_color(widgets_->voc_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(widgets_->voc_label, "%u ppb", value);
    unlock();
}

void Dashboard::updateIAQ(const std::string& dev_name, uint16_t value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->iaq_label))
        return;

    auto color = getQualityColor(value, kIaqQualityRangesIndex);
    lv_obj_set_style_text_color(widgets_->iaq_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(widgets_->iaq_label, "%u", value);
    unlock();
}

void Dashboard::updateOutsideTemperature(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->temp_outside_label, value, "--", "%+.1f%s",
                       unit_names.at(sensor_settings_->temperature));
    unlock();
}

void Dashboard::updateOutsideFeelsLike(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->feels_like_label, value, "--", "%+.1f%s",
                       unit_names.at(sensor_settings_->temperature));
    unlock();
}

void Dashboard::updateOutsideDailyHigh(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->daily_high_label, value, "--", "%.1f%s",
                       unit_names.at(sensor_settings_->temperature));
    unlock();
}

void Dashboard::updateOutsideDailyLow(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->daily_low_label, value, "--", "%.1f%s",
                       unit_names.at(sensor_settings_->temperature));
    unlock();
}

void Dashboard::updateOutsidePressure(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    float converted = convertValueToDefault(UnitType::hPa, value);
    converted       = convertValueToUnit(sensor_settings_->pressure, converted);
    updateNumericLabel(widgets_->pressure_outside_label, converted, "--", "%.0f %s",
                       unit_names.at(sensor_settings_->pressure));
    unlock();
}

void Dashboard::updateOutsideWindSpeed(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->wind_speed_label, value, "--", "%.1f %s", "m/s");
    unlock();
}

void Dashboard::updateOutsideHumidity(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->humidity_outside_label, value, "--%", "%.0f%s", "%");
    unlock();
}

void Dashboard::updateOutsidePrecipitation(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->precipitation_outside_label, value, "-- mm", "%.1f %s", "mm");
    unlock();
}

void Dashboard::updateTimeLabel(uint32_t timestamp, int32_t timestampOffset)
{
    lock();

    current_timestamp_ = timestamp;
    time_t localTimestamp = static_cast<time_t>(timestamp) + timestampOffset;

    if (widgets_)
    {
        if (localTimestamp != 0)
        {
            time_t     timestampStruct = localTimestamp;
            struct tm* timeInfo        = localtime(&timestampStruct);

            if (widgets_->label)
            {
                char time_text[6] = { 0 };
                strftime(time_text, sizeof(time_text), "%H:%M", timeInfo);
                lv_label_set_text(widgets_->label, time_text);
            }

            if (widgets_->date_label)
            {
                char date_text[24] = { 0 };
                strftime(date_text, sizeof(date_text), "%a %d %b", timeInfo);
                lv_label_set_text(widgets_->date_label, date_text);
            }
        } else
        {
            if (widgets_->label)
                lv_label_set_text(widgets_->label, "");
            if (widgets_->date_label)
                lv_label_set_text(widgets_->date_label, "");
        }
    }

    unlock();
}

namespace
{
EnvironmentalSensor::DataSample<float> unknownSample()
{
    EnvironmentalSensor::DataSample<float> sample{};
    sample.value = std::numeric_limits<float>::quiet_NaN();
    return sample;
}
} // namespace

void Dashboard::updateSensorData(const std::string& old_dev_name, const std::string& dev_name)
{
    EnvironmentalSensor::DataSample<float> temperature = unknownSample();
    EnvironmentalSensor::DataSample<float> humidity    = unknownSample();
    EnvironmentalSensor::DataSample<float> pressure    = unknownSample();
    EnvironmentalSensor::DataSample<float> co2         = unknownSample();
    EnvironmentalSensor::DataSample<float> voc         = unknownSample();
    EnvironmentalSensor::DataSample<float> iaq         = unknownSample();

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

void Dashboard::updateSettings(const std::string&)
{
    lock();
    EnvironmentalSensor::DataSample<float> temperature;
    Aggregator::instance().getTemperatureData(sensor_settings_->sensor_name, temperature);

    EnvironmentalSensor::DataSample<float> pressure;
    Aggregator::instance().getPressureData(sensor_settings_->sensor_name, pressure);

    convertToDefault(temperature);
    convertToUnit(sensor_settings_->temperature, temperature);
    updateTemperature(sensor_settings_->sensor_name, temperature.value);

    convertToDefault(pressure);
    convertToUnit(sensor_settings_->pressure, pressure);
    updatePressure(sensor_settings_->sensor_name, pressure.value);
    Aggregator::instance().setPressureData(sensor_settings_->sensor_name, pressure);

    UseCases::WeatherUpdate::instance().refresh();

    unlock();
    updateBottomPlot();
}
