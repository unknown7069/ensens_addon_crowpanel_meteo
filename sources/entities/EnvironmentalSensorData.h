#pragma once

#include <cstdint>

#include "entities/Units.h"

namespace EnvironmentalSensor
{
enum Parameters
{
    Battery = 0,
    Temperature,
    Humidity,
    Pressure,
    CO2,
    VOC,
    IAQ,
};
enum class Source : uint8_t
{
    NONE = 0,
    UART = 1 << 0,
    BLE  = 1 << 1,
    // MAX = 7 (keep values <= 0b111);
};

class Flags
{
public:
    constexpr Flags() noexcept = default;

    constexpr void set_source(Source src) noexcept
    {
        data_ = (data_ & ~Source_mask) | (static_cast<uint8_t>(src) << Source_shift);
    }

    constexpr Source source() const noexcept
    {
        return static_cast<Source>(data_ & Source_mask);
    }

    constexpr void set_history(const bool value) noexcept
    {
        data_ = (data_ & ~History_mask) | (value << History_shift);
    }

    constexpr bool is_history() const noexcept
    {
        return (data_ & History_mask) != 0;
    }

    constexpr uint8_t byte() const noexcept
    {
        return data_;
    }
    constexpr void set_byte(const uint8_t value) noexcept
    {
        data_ = value;
    }

private:
    static constexpr uint8_t Source_mask   = 0x07; // Bits 0-2 (3 bits)
    static constexpr uint8_t Source_shift  = 0;
    static constexpr uint8_t History_mask  = 0x08; // Bit 3
    static constexpr uint8_t History_shift = 3;

    uint8_t data_ = 0;
};

constexpr uint16_t paramsCount = 6;

inline const char* parameters[paramsCount] = { "Temperature", "Humidity", "Pressure",
                                               "CO2",         "VOC",      "IAQ" };

template <typename T> struct DataSample {
    uint32_t timestamp{};
    Flags    flags;
    T        value;
    UnitType unit{};
};

struct Data {
    float temperature;
    float humidity;
    float pressure;
    float co2;
    float voc;
    float iaq;
};

} // namespace EnvironmentalSensor
