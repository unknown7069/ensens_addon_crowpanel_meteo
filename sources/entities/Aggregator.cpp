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

const DailyMetricHistory*
Aggregator::getIndoorMetricHistory(const IndoorDailyMetrics& metrics,
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

DailyMetricHistory* Aggregator::getIndoorMetricHistory(IndoorDailyMetrics& metrics,
                                                       EnvironmentalSensor::Parameters param)
{
    return const_cast<DailyMetricHistory*>(
        getIndoorMetricHistory(static_cast<const IndoorDailyMetrics&>(metrics), param));
}

// Pushes a realtime sample to whichever Dashboard label shows this metric.
// The per-metric Dashboard methods differ in unit handling and quality colors,
// so dispatch on the parameter here.
static void publishMetricToDashboard(const std::string&                    dev_name,
                                     EnvironmentalSensor::Parameters       param,
                                     float                                 value)
{
    Dashboard& dashboard = Dashboard::instance();
    switch (param)
    {
    case EnvironmentalSensor::Temperature:
        dashboard.updateTemperature(dev_name, value);
        break;
    case EnvironmentalSensor::Humidity:
        dashboard.updateHumidity(dev_name, value);
        break;
    case EnvironmentalSensor::Pressure:
        dashboard.updatePressure(dev_name, value);
        break;
    case EnvironmentalSensor::CO2:
        dashboard.updateCO2(dev_name, static_cast<uint16_t>(value));
        break;
    case EnvironmentalSensor::VOC:
        dashboard.updateVOC(dev_name, static_cast<uint16_t>(value));
        break;
    case EnvironmentalSensor::IAQ:
        dashboard.updateIAQ(dev_name, static_cast<uint16_t>(value));
        break;
    default:
        break;
    }
}

template <typename SelectBuffer>
void Aggregator::handleRealtimeSample(const std::string& dev_name,
                                      EnvironmentalSensor::Parameters param,
                                      SelectBuffer select_buffer, bool apply_unit_conversion,
                                      const Sample& incoming)
{
    bool refresh_plot       = false;
    bool is_selected_sensor = false;

    {
        MutexGuard lg(mutex_);
        RealtimeData& rt_data = sensor_data_db_[dev_name];

        Sample sample = incoming;
        // Sensor values arrive in base units (Celsius/Pa); convert to the units
        // selected in settings before storing or displaying.
        if (apply_unit_conversion)
        {
            const UnitType display_unit = (param == EnvironmentalSensor::Temperature)
                                              ? sensor_settings_.temperature
                                              : sensor_settings_.pressure;
            convertToUnit(display_unit, sample);
        }

        // History replays are only kept in the daily history below, never shown
        // as realtime data.
        if (sample.flags.is_history())
            return;

        auto& buffer = select_buffer(rt_data);

        if (buffer.empty() || buffer.back_ref().value != sample.value)
            publishMetricToDashboard(dev_name, param, sample.value);

        buffer.push(sample);
        refresh_plot       = storeIndoorMetric(dev_name, param, sample);
        is_selected_sensor = (dev_name == sensor_settings_.sensor_name);
    }

    if (refresh_plot && is_selected_sensor)
    {
        Dashboard::instance().handleIndoorMetricUpdate(dev_name, param);
    }
}

bool Aggregator::storeIndoorMetric(const std::string& dev_name, EnvironmentalSensor::Parameters param,
                                   const EnvironmentalSensor::DataSample<float>& sample)
{
    if (!isIndoorSensor(dev_name))
        return false;
    if (sample.flags.is_history())
        return false;

    IndoorDailyMetrics& metrics = indoor_daily_metrics_db_[dev_name];
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
    mutex_ = xSemaphoreCreateMutex();

    ui_styles_init();
    Dashboard::instance().create(&sensor_settings_, lv_scr_act());
    return 0;
}

void Aggregator::addBatteryData(const std::string& dev_name, uint8_t battery)
{
    MutexGuard lg(mutex_);
    sensor_data_db_[dev_name].battery = battery;
}

void Aggregator::addTemperatureData(const std::string& dev_name, Sample temp)
{
    handleRealtimeSample(dev_name, EnvironmentalSensor::Temperature,
                         [](RealtimeData& d) -> auto& { return d.temperature; }, true, temp);
}

void Aggregator::addHumidityData(const std::string& dev_name, Sample humi)
{
    handleRealtimeSample(dev_name, EnvironmentalSensor::Humidity,
                         [](RealtimeData& d) -> auto& { return d.humidity; }, false, humi);
}

void Aggregator::addPressureData(const std::string& dev_name, Sample pressure)
{
    handleRealtimeSample(dev_name, EnvironmentalSensor::Pressure,
                         [](RealtimeData& d) -> auto& { return d.pressure; }, true, pressure);
}

void Aggregator::addCO2Data(const std::string& dev_name, Sample co2)
{
    handleRealtimeSample(dev_name, EnvironmentalSensor::CO2,
                         [](RealtimeData& d) -> auto& { return d.co2; }, false, co2);
}

void Aggregator::addVOCData(const std::string& dev_name, Sample voc)
{
    handleRealtimeSample(dev_name, EnvironmentalSensor::VOC,
                         [](RealtimeData& d) -> auto& { return d.voc; }, false, voc);
}

void Aggregator::addIAQData(const std::string& dev_name, Sample iaq)
{
    handleRealtimeSample(dev_name, EnvironmentalSensor::IAQ,
                         [](RealtimeData& d) -> auto& { return d.iaq; }, false, iaq);
}

void Aggregator::addDevice(const std::string& dev_name)
{
    std::vector<std::string> device_names;
    {
        MutexGuard lg(mutex_);
        if (sensor_data_db_.contains(dev_name))
            return;

        device_names.reserve(sensor_data_db_.size());
        for (const auto& [key, value] : sensor_data_db_)
        {
            device_names.push_back(key);
        }
    }
    device_names.push_back(dev_name);
    WifiScreen::instance().updateSensorNames(device_names);
}

template <typename SelectBuffer>
bool Aggregator::latestSample(const std::string& dev_name, SelectBuffer select_buffer, Sample& data)
{
    MutexGuard lg(mutex_);

    auto db_it = sensor_data_db_.find(dev_name);
    if (db_it == sensor_data_db_.end())
        return false;

    auto& buffer = select_buffer(db_it->second);
    if (buffer.empty())
        return false;

    data = buffer.back_ref();
    return true;
}

template <typename SelectBuffer>
bool Aggregator::setLatestSample(const std::string& dev_name, SelectBuffer select_buffer,
                                 const Sample& data)
{
    MutexGuard lg(mutex_);

    auto db_it = sensor_data_db_.find(dev_name);
    if (db_it == sensor_data_db_.end())
        return false;

    auto& buffer = select_buffer(db_it->second);
    if (buffer.empty())
        return false;

    buffer.back_ref() = data;
    return true;
}

template <typename SelectBuffer>
float Aggregator::latestValueOrZero(const std::string& dev_name, SelectBuffer select_buffer)
{
    Sample sample;
    return latestSample(dev_name, select_buffer, sample) ? sample.value : 0.0f;
}

bool Aggregator::getTemperatureData(const std::string& dev_name, Sample& data)
{
    return latestSample(dev_name, [](RealtimeData& d) -> auto& { return d.temperature; }, data);
}

bool Aggregator::getHumidityData(const std::string& dev_name, Sample& data)
{
    return latestSample(dev_name, [](RealtimeData& d) -> auto& { return d.humidity; }, data);
}

bool Aggregator::getPressureData(const std::string& dev_name, Sample& data)
{
    return latestSample(dev_name, [](RealtimeData& d) -> auto& { return d.pressure; }, data);
}

bool Aggregator::getCO2Data(const std::string& dev_name, Sample& data)
{
    return latestSample(dev_name, [](RealtimeData& d) -> auto& { return d.co2; }, data);
}

bool Aggregator::getVOCData(const std::string& dev_name, Sample& data)
{
    return latestSample(dev_name, [](RealtimeData& d) -> auto& { return d.voc; }, data);
}

bool Aggregator::getIAQData(const std::string& dev_name, Sample& data)
{
    return latestSample(dev_name, [](RealtimeData& d) -> auto& { return d.iaq; }, data);
}

bool Aggregator::getIndoorMetricSeries(
    const std::string& dev_name, EnvironmentalSensor::Parameters param,
    std::array<EnvironmentalSensor::DataSample<float>, DailyMetricHistory::SlotsPerDay>& slots,
    std::array<bool, DailyMetricHistory::SlotsPerDay>& has_value)
{
    MutexGuard lg(mutex_);

    auto metrics_it = indoor_daily_metrics_db_.find(dev_name);
    if (metrics_it == indoor_daily_metrics_db_.end())
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
    return latestValueOrZero(dev_name, [](RealtimeData& d) -> auto& { return d.temperature; });
}

float Aggregator::getHumidityValue(const std::string& dev_name)
{
    return latestValueOrZero(dev_name, [](RealtimeData& d) -> auto& { return d.humidity; });
}

bool Aggregator::setTemperatureData(const std::string& dev_name, const Sample& data)
{
    return setLatestSample(dev_name, [](RealtimeData& d) -> auto& { return d.temperature; }, data);
}

bool Aggregator::setHumidityData(const std::string& dev_name, const Sample& data)
{
    return setLatestSample(dev_name, [](RealtimeData& d) -> auto& { return d.humidity; }, data);
}

bool Aggregator::setPressureData(const std::string& dev_name, const Sample& data)
{
    return setLatestSample(dev_name, [](RealtimeData& d) -> auto& { return d.pressure; }, data);
}

bool Aggregator::setCO2Data(const std::string& dev_name, const Sample& data)
{
    return setLatestSample(dev_name, [](RealtimeData& d) -> auto& { return d.co2; }, data);
}

bool Aggregator::setVOCData(const std::string& dev_name, const Sample& data)
{
    return setLatestSample(dev_name, [](RealtimeData& d) -> auto& { return d.voc; }, data);
}

bool Aggregator::setIAQData(const std::string& dev_name, const Sample& data)
{
    return setLatestSample(dev_name, [](RealtimeData& d) -> auto& { return d.iaq; }, data);
}
