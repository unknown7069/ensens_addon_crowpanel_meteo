#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "entities/EnvironmentalSensorData.h"
#include "entities/BM8563.h"
#include "entities/MutexGuard.h"
#include "entities/RingBuffer.h"
#include "entities/ui/sensors_settings/SensorSettings.h"
#include "settings.h"

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#define RT_DATA_RING_BUFFER_SIZE (TENDENCY_UPDATE_PERIOD_MS / DEVICE_ADV_UPDATE_PERIOD_MS)

struct DailyMetricHistory {
    static constexpr size_t   SlotsPerDay         = 48;
    static constexpr uint32_t SlotDurationSecond = 30 * 60;

    std::array<EnvironmentalSensor::DataSample<float>, SlotsPerDay> slots     = {};
    std::array<bool, SlotsPerDay>                                   has_value{};

    int      current_year      = -1;
    int      current_yday      = -1;
    int      last_slot         = -1;
    uint32_t last_slot_updated = 0;

    bool store(const EnvironmentalSensor::DataSample<float>& sample);
};

struct IndoorDailyMetrics {
    DailyMetricHistory temperature;
    DailyMetricHistory humidity;
    DailyMetricHistory pressure;
    DailyMetricHistory co2;
    DailyMetricHistory voc;
    DailyMetricHistory iaq;
};

struct RealtimeData {
    // advertisement data is more frequent than characteristic data, therefore
    // bigger buffer
    uint8_t                                                                      battery;
    RingBuffer<EnvironmentalSensor::DataSample<float>, RT_DATA_RING_BUFFER_SIZE> temperature;
    RingBuffer<EnvironmentalSensor::DataSample<float>, RT_DATA_RING_BUFFER_SIZE> humidity;
    RingBuffer<EnvironmentalSensor::DataSample<float>, RT_DATA_RING_BUFFER_SIZE> pressure;
    RingBuffer<EnvironmentalSensor::DataSample<float>, RT_DATA_RING_BUFFER_SIZE> co2;
    RingBuffer<EnvironmentalSensor::DataSample<float>, 2>                        voc;
    RingBuffer<EnvironmentalSensor::DataSample<float>, 2>                        iaq;
};

class Aggregator
{
    using Sample = EnvironmentalSensor::DataSample<float>;

    std::unordered_map<std::string, RealtimeData>       sensor_data_db_;
    std::unordered_map<std::string, IndoorDailyMetrics> indoor_daily_metrics_db_;

    SensorSettings sensor_settings_;

    SemaphoreHandle_t mutex_ = nullptr;

    bool storeIndoorMetric(const std::string& dev_name, EnvironmentalSensor::Parameters param,
                           const Sample& sample);
    DailyMetricHistory*       getIndoorMetricHistory(IndoorDailyMetrics& metrics,
                                                     EnvironmentalSensor::Parameters param);
    const DailyMetricHistory* getIndoorMetricHistory(const IndoorDailyMetrics& metrics,
                                                     EnvironmentalSensor::Parameters param) const;
    bool isIndoorSensor(const std::string& dev_name) const;

    // Generic helpers shared by the per-metric add/get/set families. The
    // selector picks the metric's RingBuffer out of RealtimeData; it is a
    // template parameter because the six buffers differ in capacity.
    template <typename SelectBuffer>
    void handleRealtimeSample(const std::string& dev_name, EnvironmentalSensor::Parameters param,
                              SelectBuffer select_buffer, bool apply_unit_conversion,
                              const Sample& incoming);
    template <typename SelectBuffer>
    bool latestSample(const std::string& dev_name, SelectBuffer select_buffer, Sample& data);
    template <typename SelectBuffer>
    bool setLatestSample(const std::string& dev_name, SelectBuffer select_buffer,
                         const Sample& data);
    template <typename SelectBuffer>
    float latestValueOrZero(const std::string& dev_name, SelectBuffer select_buffer);

    Aggregator()                             = default;
    Aggregator(const Aggregator&)            = delete;
    Aggregator& operator=(const Aggregator&) = delete;

public:
    static Aggregator& instance()
    {
        static Aggregator instance;
        return instance;
    }
    int  create();
    void addBatteryData(const std::string& dev_name, uint8_t battery);
    void addTemperatureData(const std::string& dev_name, Sample temp);
    void addHumidityData(const std::string& dev_name, Sample humi);
    void addPressureData(const std::string& dev_name, Sample pressure);
    void addCO2Data(const std::string& dev_name, Sample co2);
    void addVOCData(const std::string& dev_name, Sample voc);
    void addIAQData(const std::string& dev_name, Sample iaq);
    void addDevice(const std::string& dev_name);

    bool getTemperatureData(const std::string& dev_name, Sample& data);
    bool getHumidityData(const std::string& dev_name, Sample& data);
    bool getPressureData(const std::string& dev_name, Sample& data);
    bool getCO2Data(const std::string& dev_name, Sample& data);
    bool getVOCData(const std::string& dev_name, Sample& data);
    bool getIAQData(const std::string& dev_name, Sample& data);

    float getTemperatureValue(const std::string& dev_name);
    float getHumidityValue(const std::string& dev_name);

    bool setTemperatureData(const std::string& dev_name, const Sample& data);
    bool setHumidityData(const std::string& dev_name, const Sample& data);
    bool setPressureData(const std::string& dev_name, const Sample& data);
    bool setCO2Data(const std::string& dev_name, const Sample& data);
    bool setVOCData(const std::string& dev_name, const Sample& data);
    bool setIAQData(const std::string& dev_name, const Sample& data);

    bool getIndoorMetricSeries(
        const std::string& dev_name, EnvironmentalSensor::Parameters param,
        std::array<EnvironmentalSensor::DataSample<float>, DailyMetricHistory::SlotsPerDay>& slots,
        std::array<bool, DailyMetricHistory::SlotsPerDay>& has_value);
};
