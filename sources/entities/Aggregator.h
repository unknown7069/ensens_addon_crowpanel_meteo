#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/projdefs.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "entities/EnvironmentalSensorData.h"
#include "entities/BM8563.h"
#include "entities/ui/sensors_settings/SensorSettings.h"
#include "settings.h"

#include <array>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "esp_log.h"

#define HIST_DATA_RING_BUFFER_SIZE HALF_HISTORY_SIZE
// buffer size required to compute tendency value
#define RT_DATA_RING_BUFFER_SIZE (TENDENCY_UPDATE_PERIOD_MS / DEVICE_ADV_UPDATE_PERIOD_MS)
// buffer size required to compute mean over certain period of time
#define MEAN_WINDOW_SIZE (HISTORY_UPDATE_PERIOD_MS / DEVICE_ADV_UPDATE_PERIOD_MS)

template <typename T, size_t N> class RingBuffer
{
    std::array<T, N> buffer = {};
    size_t           head   = 0; // Write position
    size_t           tail   = 0; // Read position
    size_t           count  = 0; // Actual element count

public:
    // Add element to buffer (overwrite oldest when full)
    // Returns "true" if the oldest was overwritten
    bool push(const T& value) noexcept
    {
        buffer[head] = value;
        head         = (head + 1) % N;

        if (count < N)
        {
            ++count;
        } else
        {
            tail = (tail + 1) % N;
            return true;
        }
        return false;
    }

    // Try to remove oldest element
    bool pop(T& output) noexcept
    {
        if (count == 0)
            return false;
        output = buffer[tail];
        tail   = (tail + 1) % N;
        --count;
        return true;
    }

    // Try to read the oldest element without removal
    bool front(T& output) const noexcept
    {
        if (count == 0)
            return false;
        output = buffer[tail];
        return true;
    }

    // Try to read the newest element without removal
    bool back(T& output) const noexcept
    {
        if (count == 0)
            return false;
        size_t last_index = (head == 0) ? (N - 1) : (head - 1);
        output            = buffer[last_index];
        return true;
    }

    // Returns a reference to the most recent element.
    // Caller must ensure the buffer is not empty before calling.
    T& back_ref()
    {
        if (count == 0)
        {
            throw std::out_of_range("RingBuffer is empty");
        }
        size_t last_index = (head == 0) ? (N - 1) : (head - 1);
        return buffer[last_index];
    }

    // Accessors
    T& at(const size_t index) noexcept
    {
        return buffer[(tail + index) % N];
    }
    const T& at(const size_t index) const noexcept
    {
        return buffer[(tail + index) % N];
    }

    T& operator[](const size_t index) noexcept
    {
        return at(index);
    }
    const T& operator[](const size_t index) const noexcept
    {
        return at(index);
    }

    // Get number of stored elements
    size_t size() const noexcept
    {
        return count;
    }

    // Check if buffer is full
    bool full() const noexcept
    {
        return count == N;
    }

    // Check if buffer is empty
    bool empty() const noexcept
    {
        return count == 0;
    }

    // Iterate elements from oldest to newest
    template <typename F> void for_each(F&& callback) noexcept
    {
        size_t current = tail;
        for (size_t i = 0; i < count; ++i)
        {
            callback(buffer[current]);
            current = (current + 1) % N;
        }
    }

    // Method to extend data starting from a specific start_point with any iterable container
    template <typename Iterable> void extend(const Iterable& new_data, size_t start_point = 0)
    {
        if (start_point >= N)
            throw std::out_of_range("start_point is out of range");

        size_t idx = start_point;
        for (const auto& value : new_data)
        {
            buffer[idx] = value;

            // Move to the next index
            idx = (idx + 1) % N; // Ensuring circular buffer

            if (count < N)
                ++count;
            else
                tail = (tail + 1) % N; // Move the tail if the buffer is full
        }

        // Update head to reflect the new position
        head = idx % N;
    }

    std::array<T, N> data()
    {
        return buffer;
    }

    void clear() noexcept
    {
        buffer.fill(T{});
        head  = 0;
        tail  = 0;
        count = 0;
    }
};

template <size_t WINDOW_SIZE> struct Mean {
private:
    double sum   = 0.0;
    size_t count = 0;

public:
    Mean() = default;

    // Returns true if value was added, false if window is full
    bool add(const float new_value)
    {
        if (full())
            return false;

        sum += static_cast<double>(new_value);
        ++count;
        return true;
    }

    // Returns calculated mean or 0.0f for empty window
    float get() const
    {
        if (count == 0)
            return 0.0f;
        return static_cast<float>(sum / count);
    }

    bool full() const
    {
        return count >= WINDOW_SIZE;
    }

    void reset()
    {
        sum   = 0.0;
        count = 0;
    }

    bool empty() const
    {
        return count == 0;
    }

    size_t size() const
    {
        return count;
    }
};

struct DailyMetricHistory {
    static constexpr size_t  SlotsPerDay         = 48;
    static constexpr uint32_t SlotDurationSecond = 30 * 60;

    std::array<EnvironmentalSensor::DataSample<float>, SlotsPerDay> slots = {};
    std::array<bool, SlotsPerDay>                                    has_value{};

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

struct HistoryData {
    RingBuffer<EnvironmentalSensor::DataSample<float>, HIST_DATA_RING_BUFFER_SIZE> pressure;
};

struct lock_guard {
    lock_guard(const SemaphoreHandle_t mutex) : mutex_(mutex)
    {
        xSemaphoreTake(mutex_, portMAX_DELAY);
    }
    ~lock_guard()
    {
        xSemaphoreGive(mutex_);
    }

private:
    SemaphoreHandle_t mutex_;
};

class PlotChartData
{
public:
    PlotChartData() {};

    float popHistoryData();
    void  popLiveData(std::vector<float>& old_values);

    void pushHistoryData(float value);
    void pushHistoryData(const std::vector<float>& new_data);
    void pushLiveData(float value);
    void pushLiveData(const std::vector<float>& new_data);

    int16_t* data()
    {
        return data_.data();
    }

    void load();

    void save(const int16_t* data, size_t data_size)
    {
        std::copy(data, data + data_size, data_.begin());
    }

    [[nodiscard]] uint16_t getHistoryDataSize() const
    {
        return history_data_size_;
    }

    [[nodiscard]] uint16_t getLiveDataSize() const
    {
        return history_data_size_;
    }

    [[nodiscard]] bool isValue(uint16_t id) const
    {
        if (id >= id_start_h_data && id < (id_end_h_data + 1))
            return id >= (id_end_h_data + 1) - history_data_size_;
        else
            return (id - (id_end_h_data + 1)) < live_data_size_;
    }

    void setSelfStorageMode(bool enable)
    {
        self_storage_mode_ = enable;
    }

private:
    bool                              self_storage_mode_ = true;
    std::array<int16_t, HISTORY_SIZE> data_              = { 0 };
    uint16_t                          history_data_size_ = 0;
    uint16_t                          live_data_size_    = 0;

    size_t id_start_h_data = 0;
    size_t id_end_h_data   = HALF_HISTORY_SIZE - 1;

    size_t id_start_l_data = HALF_HISTORY_SIZE;
    size_t id_end_l_data   = HISTORY_SIZE - 1;
};

class Aggregator
{
    std::unordered_map<std::string, RealtimeData> sensor_data_db;
    std::unordered_map<std::string, HistoryData>  history_data_db;
    std::unordered_map<std::string, IndoorDailyMetrics> indoor_daily_metrics_db;

    std::unordered_map<std::string, Mean<MEAN_WINDOW_SIZE> > pressure_mean_db;
    std::unordered_map<std::string, PlotChartData>           pressure_plot_data_db;

    SensorSettings sensor_settings;

    SemaphoreHandle_t mutex;

    bool storeIndoorMetric(const std::string& dev_name, EnvironmentalSensor::Parameters param,
                           const EnvironmentalSensor::DataSample<float>& sample);
    DailyMetricHistory*       getIndoorMetricHistory(IndoorDailyMetrics& metrics,
                                                     EnvironmentalSensor::Parameters param);
    const DailyMetricHistory* getIndoorMetricHistory(const IndoorDailyMetrics& metrics,
                                                     EnvironmentalSensor::Parameters param) const;
    bool isIndoorSensor(const std::string& dev_name) const;

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
    void addTemperatureData(const std::string&                     dev_name,
                            EnvironmentalSensor::DataSample<float> temp);
    void addHumidityData(const std::string& dev_name, EnvironmentalSensor::DataSample<float> humi);
    void addPressureData(const std::string&                     dev_name,
                         EnvironmentalSensor::DataSample<float> pressure);
    void addCO2Data(const std::string& dev_name, EnvironmentalSensor::DataSample<float> co2);
    void addVOCData(const std::string& dev_name, EnvironmentalSensor::DataSample<float> voc);
    void addIAQData(const std::string& dev_name, EnvironmentalSensor::DataSample<float> iaq);
    void addDevice(const std::string& dev_name);

    bool getTemperatureData(const std::string&                      dev_name,
                            EnvironmentalSensor::DataSample<float>& data);
    bool getHumidityData(const std::string& dev_name, EnvironmentalSensor::DataSample<float>& data);
    bool getPressureData(const std::string& dev_name, EnvironmentalSensor::DataSample<float>& data);
    bool getCO2Data(const std::string& dev_name, EnvironmentalSensor::DataSample<float>& data);
    bool getVOCData(const std::string& dev_name, EnvironmentalSensor::DataSample<float>& data);
    bool getIAQData(const std::string& dev_name, EnvironmentalSensor::DataSample<float>& data);

    float getTemperatureValue(const std::string& dev_name);
    float getHumidityValue(const std::string& dev_name);

    bool setTemperatureData(const std::string&                            dev_name,
                            const EnvironmentalSensor::DataSample<float>& data);
    bool setHumidityData(const std::string&                            dev_name,
                         const EnvironmentalSensor::DataSample<float>& data);
    bool setPressureData(const std::string&                            dev_name,
                         const EnvironmentalSensor::DataSample<float>& data);
    bool setCO2Data(const std::string&                            dev_name,
                    const EnvironmentalSensor::DataSample<float>& data);
    bool setVOCData(const std::string&                            dev_name,
                    const EnvironmentalSensor::DataSample<float>& data);
    bool setIAQData(const std::string&                            dev_name,
                    const EnvironmentalSensor::DataSample<float>& data);

    std::vector<std::string> getPressurePlotDataKeys();
    PlotChartData*           getPressurePlotData(const std::string& dev_name);
    void savePressurePlotData(const std::string& dev_name, const int16_t* data, size_t data_size);

    bool getIndoorMetricSeries(
        const std::string& dev_name, EnvironmentalSensor::Parameters param,
        std::array<EnvironmentalSensor::DataSample<float>, DailyMetricHistory::SlotsPerDay>& slots,
        std::array<bool, DailyMetricHistory::SlotsPerDay>& has_value);
};
