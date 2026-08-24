#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "portmacro.h"

#include "Aggregator.h"
#include "entities/ui/Dashboard/Dashboard.h"
#include "entities/ui/components/common.h"

#include "esp_log.h"

#include <cmath>
#include <cstdio>
#include <ctime>

static auto TAG = "Aggregator";
static constexpr const char* kIndoorSensorName = "indoor";

bool DailyMetricHistory::store(const EnvironmentalSensor::DataSample<float>& sample)
{
    if (sample.timestamp == 0)
        return false;

    time_t     raw_time = static_cast<time_t>(sample.timestamp);
    struct tm  timeinfo = {};
#if defined(_MSC_VER)
    if (localtime_s(&timeinfo, &raw_time) != 0)
        return false;
#else
    if (localtime_r(&raw_time, &timeinfo) == nullptr)
        return false;
#endif

    if (timeinfo.tm_year != current_year || timeinfo.tm_yday != current_yday)
    {
        current_year      = timeinfo.tm_year;
        current_yday      = timeinfo.tm_yday;
        has_value.fill(false);
        last_slot         = -1;
        last_slot_updated = 0;
    }

    const int minutes_since_midnight = timeinfo.tm_hour * 60 + timeinfo.tm_min;
    const int slot =
        static_cast<int>((static_cast<long>(minutes_since_midnight) * 60) / SlotDurationSecond);

    if (slot < 0 || slot >= static_cast<int>(SlotsPerDay))
        return false;

    if (!has_value[slot] || sample.timestamp >= slots[slot].timestamp)
    {
        slots[slot]         = sample;
        has_value[slot]     = true;
        last_slot           = slot;
        last_slot_updated   = sample.timestamp;
        return true;
    }

    return false;
}

DailyMetricHistory* Aggregator::getIndoorMetricHistory(IndoorDailyMetrics& metrics,
                                                       EnvironmentalSensor::Parameters param)
{
    switch (param)
    {
    case EnvironmentalSensor::Temperature:
        return &metrics.temperature;
    case EnvironmentalSensor::Humidity:
        return &metrics.humidity;
    case EnvironmentalSensor::Pressure:
        return &metrics.pressure;
    case EnvironmentalSensor::CO2:
        return &metrics.co2;
    case EnvironmentalSensor::VOC:
        return &metrics.voc;
    case EnvironmentalSensor::IAQ:
        return &metrics.iaq;
    default:
        return nullptr;
    }
}

const DailyMetricHistory* Aggregator::getIndoorMetricHistory(const IndoorDailyMetrics& metrics,
                                                             EnvironmentalSensor::Parameters param) const
{
    switch (param)
    {
    case EnvironmentalSensor::Temperature:
        return &metrics.temperature;
    case EnvironmentalSensor::Humidity:
        return &metrics.humidity;
    case EnvironmentalSensor::Pressure:
        return &metrics.pressure;
    case EnvironmentalSensor::CO2:
        return &metrics.co2;
    case EnvironmentalSensor::VOC:
        return &metrics.voc;
    case EnvironmentalSensor::IAQ:
        return &metrics.iaq;
    default:
        return nullptr;
    }
}

bool Aggregator::storeIndoorMetric(const std::string& dev_name, EnvironmentalSensor::Parameters param,
                                   const EnvironmentalSensor::DataSample<float>& sample)
{
    if (!isIndoorSensor(dev_name))
        return false;
    if (sample.flags.is_history())
        return false;

    IndoorDailyMetrics& metrics = indoor_daily_metrics_db[dev_name];
    DailyMetricHistory* history = getIndoorMetricHistory(metrics, param);
    if (history == nullptr)
        return false;

    EnvironmentalSensor::DataSample<float> adjusted_sample = sample;
    uint32_t dashboard_ts = Dashboard::instance().getCurrentTimestamp();
    if (dashboard_ts != 0)
        adjusted_sample.timestamp = dashboard_ts;

    return history->store(adjusted_sample);
}

bool Aggregator::isIndoorSensor(const std::string& dev_name) const
{
    return dev_name == kIndoorSensorName;
}

int Aggregator::create()
{
    ESP_LOGD(TAG, "RT_DATA_RING_BUFFER_SIZE=%d", RT_DATA_RING_BUFFER_SIZE);
    mutex = xSemaphoreCreateMutex();

    ui_styles_init();
    Dashboard::instance().create(&sensor_settings, lv_scr_act());
    return 0;
}

void Aggregator::addBatteryData(const std::string& dev_name, uint8_t battery)
{
    lock_guard lg(mutex);
    sensor_data_db[dev_name].battery = battery;
}

void Aggregator::addTemperatureData(const std::string&                     dev_name,
                                    EnvironmentalSensor::DataSample<float> temp)
{
    bool refresh_plot       = false;
    bool is_selected_sensor = false;

    {
        lock_guard                             lg(mutex);
        RealtimeData&                          rt_data    = sensor_data_db[dev_name];

        convertToUnit(sensor_settings.temperature, temp);

        if (rt_data.temperature.empty() || rt_data.temperature.back_ref().value != temp.value)
        {
            Dashboard::instance().updateTemperature(dev_name, temp.value);
        }

        rt_data.temperature.push(temp);
        refresh_plot       = storeIndoorMetric(dev_name, EnvironmentalSensor::Temperature, temp);
        is_selected_sensor = (dev_name == sensor_settings.sensor_name);
    }

    if (refresh_plot && is_selected_sensor)
    {
        Dashboard::instance().handleIndoorMetricUpdate(dev_name, EnvironmentalSensor::Temperature);
    }
}

void Aggregator::addHumidityData(const std::string&                     dev_name,
                                 EnvironmentalSensor::DataSample<float> humi)
{
    bool refresh_plot       = false;
    bool is_selected_sensor = false;

    {
        lock_guard                             lg(mutex);
        RealtimeData&                          rt_data    = sensor_data_db[dev_name];

        if (rt_data.humidity.empty() || rt_data.humidity.back_ref().value != humi.value)
        {
            Dashboard::instance().updateHumidity(dev_name, humi.value);
        }

        rt_data.humidity.push(humi);
        refresh_plot       = storeIndoorMetric(dev_name, EnvironmentalSensor::Humidity, humi);
        is_selected_sensor = (dev_name == sensor_settings.sensor_name);
    }

    if (refresh_plot && is_selected_sensor)
    {
        Dashboard::instance().handleIndoorMetricUpdate(dev_name, EnvironmentalSensor::Humidity);
    }
}

void Aggregator::addPressureData(const std::string&                     dev_name,
                                 EnvironmentalSensor::DataSample<float> pressure)
{
    bool refresh_plot       = false;
    bool is_selected_sensor = false;

    {
        lock_guard    lg(mutex);
        RealtimeData& rt_data = sensor_data_db[dev_name];

        convertToUnit(sensor_settings.pressure, pressure);

        if (pressure.flags.is_history())
        {
            return;
        }

        if (rt_data.pressure.empty() || rt_data.pressure.back_ref().value != pressure.value)
        {
            Dashboard::instance().updatePressure(dev_name, pressure.value);
        }

        rt_data.pressure.push(pressure);
        refresh_plot       = storeIndoorMetric(dev_name, EnvironmentalSensor::Pressure, pressure);
        is_selected_sensor = (dev_name == sensor_settings.sensor_name);
    }

    if (refresh_plot && is_selected_sensor)
    {
        Dashboard::instance().handleIndoorMetricUpdate(dev_name, EnvironmentalSensor::Pressure);
    }
}

void Aggregator::addCO2Data(const std::string& dev_name, EnvironmentalSensor::DataSample<float> co2)
{
    bool refresh_plot       = false;
    bool is_selected_sensor = false;

    {
        lock_guard                             lg(mutex);
        RealtimeData&                          rt_data    = sensor_data_db[dev_name];
        if (rt_data.co2.empty() || rt_data.co2.back_ref().value != co2.value)
        {
            Dashboard::instance().updateCO2(dev_name, co2.value);
        }

        rt_data.co2.push(co2);
        refresh_plot       = storeIndoorMetric(dev_name, EnvironmentalSensor::CO2, co2);
        is_selected_sensor = (dev_name == sensor_settings.sensor_name);
    }

    if (refresh_plot && is_selected_sensor)
    {
        Dashboard::instance().handleIndoorMetricUpdate(dev_name, EnvironmentalSensor::CO2);
    }
}

void Aggregator::addVOCData(const std::string& dev_name, EnvironmentalSensor::DataSample<float> voc)
{
    bool refresh_plot       = false;
    bool is_selected_sensor = false;

    {
        lock_guard                             lg(mutex);
        RealtimeData&                          rt_data    = sensor_data_db[dev_name];
        if (rt_data.voc.empty() || rt_data.voc.back_ref().value != voc.value)
        {
            Dashboard::instance().updateVOC(dev_name, voc.value);
        }

        rt_data.voc.push(voc);
        refresh_plot       = storeIndoorMetric(dev_name, EnvironmentalSensor::VOC, voc);
        is_selected_sensor = (dev_name == sensor_settings.sensor_name);
    }

    if (refresh_plot && is_selected_sensor)
    {
        Dashboard::instance().handleIndoorMetricUpdate(dev_name, EnvironmentalSensor::VOC);
    }
}

void Aggregator::addIAQData(const std::string& dev_name, EnvironmentalSensor::DataSample<float> iaq)
{
    bool refresh_plot       = false;
    bool is_selected_sensor = false;

    {
        lock_guard                             lg(mutex);
        RealtimeData&                          rt_data    = sensor_data_db[dev_name];
        if (rt_data.iaq.empty() || rt_data.iaq.back_ref().value != iaq.value)
        {
            Dashboard::instance().updateIAQ(dev_name, iaq.value);
        }

        rt_data.iaq.push(iaq);
        refresh_plot       = storeIndoorMetric(dev_name, EnvironmentalSensor::IAQ, iaq);
        is_selected_sensor = (dev_name == sensor_settings.sensor_name);
    }

    if (refresh_plot && is_selected_sensor)
    {
        Dashboard::instance().handleIndoorMetricUpdate(dev_name, EnvironmentalSensor::IAQ);
    }
}

void Aggregator::addDevice(const std::string& dev_name)
{
    if (sensor_data_db.contains(dev_name))
        return;

    std::vector<std::string> device_names;
    device_names.reserve(sensor_data_db.size() + 1);
    for (const auto& [key, value] : sensor_data_db)
    {
        device_names.push_back(key);
    }
    device_names.push_back(dev_name);
    WifiScreen::instance().updateSensorNames(device_names);
}

bool Aggregator::getTemperatureData(const std::string&                      dev_name,
                                    EnvironmentalSensor::DataSample<float>& data)
{
    auto temperature = sensor_data_db[dev_name].temperature;
    if (temperature.empty())
        return false;
    data = temperature.back_ref();
    return true;
}

bool Aggregator::getHumidityData(const std::string&                      dev_name,
                                 EnvironmentalSensor::DataSample<float>& data)
{
    auto humidity = sensor_data_db[dev_name].humidity;
    if (humidity.empty())
        return false;
    data = humidity.back_ref();
    return true;
}

bool Aggregator::getPressureData(const std::string&                      dev_name,
                                 EnvironmentalSensor::DataSample<float>& data)
{
    auto pressure = sensor_data_db[dev_name].pressure;
    if (pressure.empty())
        return false;
    data = pressure.back_ref();
    return true;
}

bool Aggregator::getCO2Data(const std::string&                      dev_name,
                            EnvironmentalSensor::DataSample<float>& data)
{
    auto co2 = sensor_data_db[dev_name].co2;
    if (co2.empty())
        return false;
    data = co2.back_ref();
    return true;
}

bool Aggregator::getVOCData(const std::string&                      dev_name,
                            EnvironmentalSensor::DataSample<float>& data)
{
    auto voc = sensor_data_db[dev_name].voc;
    if (voc.empty())
        return false;
    data = voc.back_ref();
    return true;
}

bool Aggregator::getIAQData(const std::string&                      dev_name,
                            EnvironmentalSensor::DataSample<float>& data)
{
    auto iaq = sensor_data_db[dev_name].iaq;
    if (iaq.empty())
        return false;
    data = iaq.back_ref();
    return true;
}

bool Aggregator::getIndoorMetricSeries(
    const std::string& dev_name, EnvironmentalSensor::Parameters param,
    std::array<EnvironmentalSensor::DataSample<float>, DailyMetricHistory::SlotsPerDay>& slots,
    std::array<bool, DailyMetricHistory::SlotsPerDay>& has_value)
{
    lock_guard lg(mutex);

    auto metrics_it = indoor_daily_metrics_db.find(dev_name);
    if (metrics_it == indoor_daily_metrics_db.end())
        return false;

    const DailyMetricHistory* history = getIndoorMetricHistory(metrics_it->second, param);
    if (history == nullptr)
        return false;

    slots     = history->slots;
    has_value = history->has_value;
    return true;
}

float Aggregator::getTemperatureValue(const std::string& dev_name)
{
    auto temperature = sensor_data_db[dev_name].temperature;
    if (temperature.empty())
        return 0;
    return temperature.back_ref().value;
}

float Aggregator::getHumidityValue(const std::string& dev_name)
{
    auto humidity = sensor_data_db[dev_name].humidity;
    if (humidity.empty())
        return 0;
    return humidity.back_ref().value;
}

bool Aggregator::setTemperatureData(const std::string&                            dev_name,
                                    const EnvironmentalSensor::DataSample<float>& data)
{
    if (sensor_data_db[dev_name].temperature.empty())
        return false;
    sensor_data_db[dev_name].temperature.back_ref() = data;
    return true;
}

bool Aggregator::setHumidityData(const std::string&                            dev_name,
                                 const EnvironmentalSensor::DataSample<float>& data)
{
    if (sensor_data_db[dev_name].humidity.empty())
        return false;
    sensor_data_db[dev_name].humidity.back_ref() = data;
    return true;
}

bool Aggregator::setPressureData(const std::string&                            dev_name,
                                 const EnvironmentalSensor::DataSample<float>& data)
{
    if (sensor_data_db[dev_name].pressure.empty())
        return false;
    sensor_data_db[dev_name].pressure.back_ref() = data;
    return true;
}

bool Aggregator::setCO2Data(const std::string&                            dev_name,
                            const EnvironmentalSensor::DataSample<float>& data)
{
    if (sensor_data_db[dev_name].co2.empty())
        return false;
    sensor_data_db[dev_name].co2.back_ref() = data;
    return true;
}

bool Aggregator::setVOCData(const std::string&                            dev_name,
                            const EnvironmentalSensor::DataSample<float>& data)
{
    if (sensor_data_db[dev_name].voc.empty())
        return false;
    sensor_data_db[dev_name].voc.back_ref() = data;
    return true;
}

bool Aggregator::setIAQData(const std::string&                            dev_name,
                            const EnvironmentalSensor::DataSample<float>& data)
{
    if (sensor_data_db[dev_name].iaq.empty())
        return false;
    sensor_data_db[dev_name].iaq.back_ref() = data;
    return true;
}

