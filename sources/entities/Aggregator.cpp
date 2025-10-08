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

    return history->store(sample);
}

bool Aggregator::isIndoorSensor(const std::string& dev_name) const
{
    return dev_name == kIndoorSensorName;
}

void PlotChartData::pushHistoryData(float value)
{
    popHistoryData();

    size_t idx = id_end_h_data;

    if (self_storage_mode_)
        data_[idx] = static_cast<int16_t>(value);
    else
        Dashboard::instance().setPressurePlotValue(static_cast<int16_t>(value), idx);
    history_data_size_ += 2;
}

void PlotChartData::pushHistoryData(const std::vector<float>& new_data)
{
    for (auto& value : new_data)
        pushHistoryData(value);
}

void PlotChartData::pushLiveData(float value)
{
    Dashboard::instance().updatePressurePlot(&value, 1);
}

void PlotChartData::pushLiveData(const std::vector<float>& new_data)
{
    for (auto& value : new_data)
        pushLiveData(value);
}

float PlotChartData::popHistoryData()
{
    uint16_t data_size = Dashboard::instance().getPressurePlotRawDataSize();
    if (data_size < id_start_h_data)
        throw std::runtime_error("Index out of range");

    int16_t* data;
    if (self_storage_mode_)
        data = data_.data();
    else
    {
        data = Dashboard::instance().getPressurePlotRawData();
    }
    int16_t value = data[0];

    for (size_t i = id_start_h_data; i <= id_end_h_data; i++)
    {
        if (self_storage_mode_)
            data_[i] = static_cast<int16_t>(data[i + 1]);
        else
            Dashboard::instance().setPressurePlotValue(data[i + 1], i);
    }

    history_data_size_--;
    return static_cast<float>(value);
}

void PlotChartData::popLiveData(std::vector<float>& old_values)
{
    uint16_t data_size = Dashboard::instance().getPressurePlotRawDataSize();
    if (data_size < id_start_l_data)
        throw std::runtime_error("Index out of range");

    int16_t* data;
    if (self_storage_mode_)
        data = data_.data();
    else
        data = Dashboard::instance().getPressurePlotRawData();

    for (size_t i = id_start_l_data; i <= id_end_l_data; i++)
    {
        old_values.push_back(static_cast<float>(data[i]));

        if (self_storage_mode_)
        {
            data_[i] = static_cast<int16_t>(0);
        } else
        {
            Dashboard::instance().setPressurePlotValue(static_cast<int16_t>(0), i);
        }
    }
    live_data_size_ = 0;
}

void PlotChartData::load()
{
    for (auto i = 0; i < data_.size(); i++)
        Dashboard::instance().setPressurePlotValue(data_[i], i);
}

int Aggregator::create()
{
    ESP_LOGD(TAG,
             "HIST_DATA_RING_BUFFER_SIZE=%d, RT_DATA_RING_BUFFER_SIZE=%d, "
             "MEAN_WINDOW_SIZE=%d",
             HISTORY_SIZE, RT_DATA_RING_BUFFER_SIZE, MEAN_WINDOW_SIZE);
    mutex = xSemaphoreCreateMutex();

    ui_styles_init();
    Dashboard::instance().create(&sensor_settings, lv_scr_act());
    Dashboard::instance().updatePressureBox(sensor_settings.pressure);
    return 0;
}

static uint8_t convert_temp_diff(float diff)
{
    if (diff == 0.f)
        return 0;
    if (diff < 0.01f)
        return 1;
    if (diff < 0.2f)
        return 2;
    if (diff < 0.4f)
        return 3;
    if (diff < 0.6f)
        return 4;
    return 5;
}

static uint8_t convert_humi_diff(float diff)
{
    if (diff == 0.f)
        return 0;
    if (diff < 0.01f)
        return 1;
    if (diff < 0.2f)
        return 2;
    if (diff < 0.4f)
        return 3;
    if (diff < 0.6f)
        return 4;
    return 5;
}

static uint8_t convert_pressure_diff(float diff)
{
    if (diff == 0.f)
        return 0;
    if (diff < 0.01f)
        return 1;
    if (diff < 0.1f)
        return 2;
    if (diff < 0.21f)
        return 3;
    if (diff < 0.32f)
        return 4;
    return 5;
}

static uint8_t convert_co2_diff(float diff)
{
    if (diff == 0.f)
        return 0;
    if (diff < 0.5f)
        return 1;
    if (diff < 2.5f)
        return 2;
    if (diff < 5.f)
        return 3;
    if (diff < 7.5f)
        return 4;
    return 5;
}

static uint8_t convert_voc_diff(float diff)
{
    if (diff == 0.f)
        return 0;
    if (diff < 0.02f)
        return 1;
    if (diff < 1.25f)
        return 2;
    if (diff < 2.5f)
        return 3;
    if (diff < 3.75f)
        return 4;
    return 5;
}

static uint8_t convert_iaq_diff(float diff)
{
    if (diff == 0.f)
        return 0;
    if (diff < 0.1f)
        return 1;
    if (diff < 1.f)
        return 2;
    if (diff < 2.f)
        return 3;
    if (diff < 3.f)
        return 4;
    return 5;
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
        EnvironmentalSensor::DataSample<float> old_sample = {};

        convertToUnit(sensor_settings.temperature, temp);

        if (rt_data.temperature.full() && rt_data.temperature.front(old_sample))
        {
            float diff = temp.value - old_sample.value;
            ESP_LOGD(TAG, "temp: diff=%.2f", diff);
            Dashboard::instance().updateTemperatureTendency(convert_temp_diff(fabs(diff)),
                                                            diff > 0.f ? 1 : -1);
        }

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
        EnvironmentalSensor::DataSample<float> old_sample = {};

        if (rt_data.humidity.full() && rt_data.humidity.front(old_sample))
        {
            float diff = humi.value - old_sample.value;
            ESP_LOGD(TAG, "humi: diff=%.2f", diff);
            Dashboard::instance().updateHumidityTendency(convert_humi_diff(fabs(diff)),
                                                         diff > 0.f ? 1 : -1);
        }

        if (rt_data.humidity.empty() || rt_data.humidity.back_ref().value != humi.value)
        {
            Dashboard::instance().updateHumidity(dev_name, humi.value);
            Dashboard::instance().updateDewPoint(dev_name);
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
        lock_guard              lg(mutex);
        RealtimeData&           rt_data       = sensor_data_db[dev_name];
        HistoryData&            h_data        = history_data_db[dev_name];
        PlotChartData&          chart_data    = pressure_plot_data_db[dev_name];
        Mean<MEAN_WINDOW_SIZE>& pressure_mean = pressure_mean_db[dev_name];

        if (dev_name == sensor_settings.sensor_name)
            chart_data.setSelfStorageMode(false);

        convertToUnit(sensor_settings.pressure, pressure);

        if (pressure.flags.is_history())
        {
            h_data.pressure.push(pressure);
            chart_data.pushHistoryData(pressure.value);
            return;
        }
        EnvironmentalSensor::DataSample<float> old_sample = {};

        if (rt_data.pressure.full() && rt_data.pressure.front(old_sample))
        {
            if (old_sample.unit != pressure.unit)
                convertToUnit(sensor_settings.pressure, old_sample);
            float diff = pressure.value - old_sample.value;
            ESP_LOGD(TAG, "pressure: diff=%.2f", diff);
            Dashboard::instance().updatePressureTendency(convert_pressure_diff(fabs(diff)),
                                                         diff > 0.f ? 1 : -1);
        }
        if (!pressure_mean.add(pressure.value))
        {
            // Now "mean" value isn't used
            // Instead of it is used "runtime" value from sensor

            float mean = pressure_mean.get();

            EnvironmentalSensor::Flags flags;
            flags.set_source(EnvironmentalSensor::Source::NONE);
            time_t time_val = time(nullptr);
            auto   t        = static_cast<uint32_t>(time_val);
            // ESP_LOGD(TAG, "pressure: dev_name=%s, time=%lu, mean=%.2f", dev_name.c_str(), t, mean);
            ESP_LOGD(TAG, "pressure: dev_name=%s, time=%lu, mean=%.2f", dev_name.c_str(), t,
                     pressure.value);
            // h_data.pressure.push({
            //     .timestamp = t,
            //     .flags     = flags,
            //     .value     = mean,
            // });
            h_data.pressure.push({
                .timestamp = t,
                .flags     = flags,
                .value     = pressure.value,
            });
            // chart_data.pushLiveData(mean);
            chart_data.pushLiveData(pressure.value);
            pressure_mean.reset();
            pressure_mean.add(pressure.value);
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
        EnvironmentalSensor::DataSample<float> old_sample = {};
        if (rt_data.co2.full() && rt_data.co2.front(old_sample))
        {
            float diff = co2.value - old_sample.value;
            ESP_LOGD(TAG, "co2: diff=%.2f", diff);
            Dashboard::instance().updateCO2Tendency(convert_co2_diff(fabs(diff)),
                                                     diff > 0.f ? 1 : -1);
        }

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
        EnvironmentalSensor::DataSample<float> old_sample = {};
        if (rt_data.voc.full() && rt_data.voc.front(old_sample))
        {
            float diff = voc.value - old_sample.value;
            ESP_LOGD(TAG, "voc: diff=%.2f", diff);
            Dashboard::instance().updateVOCTendency(convert_voc_diff(fabs(diff)),
                                                     diff > 0.f ? 1 : -1);
        }

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
        EnvironmentalSensor::DataSample<float> old_sample = {};
        if (rt_data.iaq.full() && rt_data.iaq.front(old_sample))
        {
            float diff = iaq.value - old_sample.value;
            ESP_LOGD(TAG, "iaq: diff=%.2f", diff);
            Dashboard::instance().updateIAQTendency(convert_iaq_diff(fabs(diff)),
                                                     diff > 0.f ? 1 : -1);
        }

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

std::vector<std::string> Aggregator::getPressurePlotDataKeys()
{
    std::vector<std::string> keys;
    for (auto& i : pressure_plot_data_db)
        keys.push_back(i.first);
    return keys;
}

PlotChartData* Aggregator::getPressurePlotData(const std::string& dev_name)
{
    if (!pressure_plot_data_db.contains(dev_name))
        return nullptr;

    return &pressure_plot_data_db[dev_name];
}

void Aggregator::savePressurePlotData(const std::string& dev_name, const int16_t* data,
                                      size_t data_size)
{
    auto& plot_data = pressure_plot_data_db[dev_name];
    plot_data.save(data, data_size);
    plot_data.setSelfStorageMode(true);
}
