#include "Dashboard.h"

#include "usecases/WeatherUpdate.h"
#include "KitchenTheme.h"
#include "WeatherIcon.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <limits>

LV_FONT_DECLARE(kitchen_temperature_88);

lv_obj_t* Dashboard::create(SensorSettings* sensor_settings, lv_obj_t* parent)
{
    lock();
    sensor_settings_ = sensor_settings;

    widgets_ = dashboard_view_create(parent, 0);
    if (!widgets_)
    {
        unlock();
        return nullptr;
    }
    WifiScreen::instance().create(sensor_settings_, widgets_->tab_settings);
    WifiScreen::instance().loadSettings();

    unlock();
    updateStatusCharts();
    return widgets_->cont;
}

void Dashboard::copyStatusMeasurement(StatusMetric metric, lv_obj_t* source)
{
    lv_obj_t* target = widgets_->status_metrics[static_cast<size_t>(metric)].value;
    if (target == source)
        return;
    lv_label_set_text(target, lv_label_get_text(source));
    lv_obj_set_style_text_color(target, lv_obj_get_style_text_color(source, LV_PART_MAIN), 0);
}

void Dashboard::updateStatusCharts()
{
    for (size_t i = 0; i < 6; ++i)
        updateStatusChart(static_cast<StatusMetric>(i));
}

void Dashboard::refreshIndoorHistory()
{
    // Called without the LVGL lock: the aggregator must be read first.
    updateStatusCharts();
}

void Dashboard::updateStatusChart(StatusMetric metric)
{
    if (widgets_ == nullptr || sensor_settings_ == nullptr)
        return;

    // Read history before taking the LVGL lock: sensor publishing also takes
    // these locks, in aggregator-then-LVGL order.
    IndoorMetricHistory::Samples samples{};
    IndoorMetricHistory::Validity has_value{};
    const uint32_t now = getCurrentTimestamp();
    const std::string sensor_name = sensor_settings_->sensor_name;
    const bool available = Aggregator::instance().getIndoorMetricSeries(
        sensor_name, parameterFromMetric(metric), now, samples, has_value);

    lock();
    if (sensor_name != sensor_settings_->sensor_name)
    {
        unlock();
        return;
    }
    auto& row = widgets_->status_metrics[static_cast<size_t>(metric)];
    std::array<float, IndoorMetricHistory::PointCount> values{};
    float minimum = std::numeric_limits<float>::max();
    float maximum = std::numeric_limits<float>::lowest();
    bool any = false;
    for (size_t i = 0; i < samples.size(); ++i)
    {
        values[i] = std::numeric_limits<float>::quiet_NaN();
        if (available && has_value[i])
        {
            auto sample = samples[i];
            if (metric == StatusMetric::Temperature || metric == StatusMetric::Pressure)
            {
                convertToDefault(sample);
                convertToUnit(metric == StatusMetric::Temperature ? sensor_settings_->temperature : sensor_settings_->pressure, sample);
            }
            if (std::isfinite(sample.value))
            {
                float value = sample.value;
                minimum = std::min(minimum, value);
                maximum = std::max(maximum, value);
                values[i] = value;
                any = true;
            }
        }
    }
    // Sparklines have no numeric axis: normalize their range to retain small
    // changes and keep readings such as pressure in Pa within LVGL's 16-bit coordinates.
    const float range = any ? maximum - minimum : 0.f;
    for (size_t i = 0; i < values.size(); ++i)
    {
        const lv_coord_t point = !std::isfinite(values[i]) ? LV_CHART_POINT_NONE
            : range > 0.f ? static_cast<lv_coord_t>(std::lround((values[i] - minimum) / range * 1000.f))
                          : 500;
        lv_chart_set_value_by_id(row.chart, row.series, static_cast<uint16_t>(i), point);
    }
    lv_chart_set_range(row.chart, LV_CHART_AXIS_PRIMARY_Y, -100, 1100);
    lv_chart_refresh(row.chart);
    unlock();
}

bool Dashboard::mapParameterToMetric(EnvironmentalSensor::Parameters param,
                                     StatusMetric& metric) const
{
    switch (param)
    {
    case EnvironmentalSensor::Temperature:
        metric = StatusMetric::Temperature;
        return true;
    case EnvironmentalSensor::Humidity:
        metric = StatusMetric::Humidity;
        return true;
    case EnvironmentalSensor::Pressure:
        metric = StatusMetric::Pressure;
        return true;
    case EnvironmentalSensor::CO2:
        metric = StatusMetric::CO2;
        return true;
    case EnvironmentalSensor::VOC:
        metric = StatusMetric::VOC;
        return true;
    case EnvironmentalSensor::IAQ:
        metric = StatusMetric::IAQ;
        return true;
    default:
        return false;
    }
}

EnvironmentalSensor::Parameters Dashboard::parameterFromMetric(StatusMetric metric) const
{
    switch (metric)
    {
    case StatusMetric::Temperature:
        return EnvironmentalSensor::Temperature;
    case StatusMetric::Humidity:
        return EnvironmentalSensor::Humidity;
    case StatusMetric::Pressure:
        return EnvironmentalSensor::Pressure;
    case StatusMetric::CO2:
        return EnvironmentalSensor::CO2;
    case StatusMetric::VOC:
        return EnvironmentalSensor::VOC;
    case StatusMetric::IAQ:
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

    StatusMetric metric;
    if (!mapParameterToMetric(param, metric))
        return;

    updateStatusChart(metric);
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

    if (!std::isfinite(value))
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

    updateNumericLabel(widgets_->temp_inside_label, value, "--°", "%.0f%s", "°");
    updateNumericLabel(widgets_->status_metrics[0].value, value, "--", "%.1f%s",
                       unit_names.at(sensor_settings_->temperature));
    unlock();
}

void Dashboard::updateHumidity(const std::string& dev_name, float value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->humidity_inside_label))
        return;

    updateNumericLabel(widgets_->humidity_inside_label, value, "--%", "%.0f%s", "%");
    updateNumericLabel(widgets_->indoor_humidity_label, value, "--%", "%.0f%s", "%");
    copyStatusMeasurement(StatusMetric::Humidity, widgets_->humidity_inside_label);
    unlock();
}

void Dashboard::updatePressure(const std::string& dev_name, float value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->pressure_inside_label))
        return;

    updateNumericLabel(widgets_->pressure_inside_label, value, "--", "%.0f %s",
                       unit_names.at(sensor_settings_->pressure));
    copyStatusMeasurement(StatusMetric::Pressure, widgets_->pressure_inside_label);
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

void Dashboard::updateCO2(const std::string& dev_name, float value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->co2_label))
        return;

    auto color = std::isfinite(value) ? getQualityColor(static_cast<uint16_t>(std::clamp(value, 0.f, 65535.f)), kCo2QualityRangesPpm) : lv_palette_main(LV_PALETTE_GREY);
    lv_obj_set_style_text_color(widgets_->co2_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    updateNumericLabel(widgets_->co2_label, value, "--", "%.0f%s", " ppm");
    copyStatusMeasurement(StatusMetric::CO2, widgets_->co2_label);
    unlock();
}

void Dashboard::updateVOC(const std::string& dev_name, float value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->voc_label))
        return;

    auto color = std::isfinite(value) ? getQualityColor(static_cast<uint16_t>(std::clamp(value, 0.f, 65535.f)), kVocQualityRangesPpb) : lv_palette_main(LV_PALETTE_GREY);
    lv_obj_set_style_text_color(widgets_->voc_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    updateNumericLabel(widgets_->voc_label, value, "--", "%.0f%s", " ppb");
    copyStatusMeasurement(StatusMetric::VOC, widgets_->voc_label);
    unlock();
}

void Dashboard::updateIAQ(const std::string& dev_name, float value)
{
    if (widgets_ == nullptr || !beginSelectedLabelUpdate(dev_name, widgets_->iaq_label))
        return;

    auto color = std::isfinite(value) ? getQualityColor(static_cast<uint16_t>(std::clamp(value, 0.f, 65535.f)), kIaqQualityRangesIndex) : lv_palette_main(LV_PALETTE_GREY);
    lv_obj_set_style_text_color(widgets_->iaq_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
    updateNumericLabel(widgets_->iaq_label, value, "--", "%.0f%s", "");
    copyStatusMeasurement(StatusMetric::IAQ, widgets_->iaq_label);
    unlock();
}

void Dashboard::updateOutsideTemperature(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->temp_outside_label,
                       convertValueToUnit(sensor_settings_->temperature, value), "--°", "%.0f%s", "°");
    lv_point_t size;
    lv_txt_get_size(&size, lv_label_get_text(widgets_->temp_outside_label),
                    &kitchen_temperature_88, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    lv_obj_set_style_text_font(widgets_->temp_outside_label,
                               size.x > 144 ? &lv_font_montserrat_48 : &kitchen_temperature_88, 0);
    unlock();
}

void Dashboard::updateOutsideFeelsLike(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->feels_like_label,
                       convertValueToUnit(sensor_settings_->temperature, value), "Feels like --", "Feels like %.0f%s",
                       unit_names.at(sensor_settings_->temperature));
    unlock();
}

void Dashboard::updateOutsideDailyHigh(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->daily_high_label, convertValueToUnit(sensor_settings_->temperature, value), "--", "%+.1f%s",
                       unit_names.at(sensor_settings_->temperature));
    unlock();
}

void Dashboard::updateOutsideDailyLow(float value)
{
    if (widgets_ == nullptr)
        return;
    lock();
    updateNumericLabel(widgets_->daily_low_label, convertValueToUnit(sensor_settings_->temperature, value), "--", "%+.1f%s",
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
    timezone_offset_ = timestampOffset;
    if (widgets_ && timestamp)
    {
        const int64_t local = static_cast<int64_t>(timestamp) + timestampOffset;
        dashboard_view_update_clock(widgets_, local);
        dashboard_view_update_forecast(widgets_, forecast_, local,
                                        sensor_settings_->temperature == Fahrenheit);
    }
    unlock();
}

void Dashboard::updateForecast(const WeatherForecast& forecast, const char* currentIcon)
{
    lock();
    forecast_ = forecast;
    if (widgets_)
    {
        weather_icon_set(widgets_->outside_icon, currentIcon);
        const int64_t local = current_timestamp_ ?
            static_cast<int64_t>(current_timestamp_) + timezone_offset_ : forecast.referenceTime;
        dashboard_view_update_forecast(widgets_, forecast_, local,
                                        sensor_settings_->temperature == Fahrenheit);
    }
    unlock();
}

void Dashboard::updateWifiStatus(bool connected)
{
    lock();
    if (widgets_)
        lv_obj_set_style_text_opa(widgets_->wifi_label, connected ? LV_OPA_COVER : LV_OPA_40, 0);
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
    EnvironmentalSensor::DataSample<float> temperature = unknownSample();
    Aggregator::instance().getTemperatureData(sensor_settings_->sensor_name, temperature);

    EnvironmentalSensor::DataSample<float> pressure = unknownSample();
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
    updateStatusCharts();
}
