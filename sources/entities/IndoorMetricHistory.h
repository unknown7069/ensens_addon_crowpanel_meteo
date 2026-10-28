#pragma once

#include "EnvironmentalSensorData.h"

#include <array>
#include <cstddef>

// A rolling day of readings, independent of calendar dates and timezones.
// Keep the latest reading in each absolute half-hour bucket. The two partial
// end buckets require 49 entries to cover a full 24 hours without losing the
// oldest half hour as new readings arrive.
class IndoorMetricHistory {
public:
    static constexpr uint32_t WindowSeconds = 24 * 60 * 60;
    static constexpr uint32_t BucketSeconds = 30 * 60;
    static constexpr size_t PointCount = WindowSeconds / BucketSeconds + 1;
    using Sample = EnvironmentalSensor::DataSample<float>;
    using Samples = std::array<Sample, PointCount>;
    using Validity = std::array<bool, PointCount>;

    bool store(const Sample& sample)
    {
        if (!sample.timestamp)
            return false;
        auto& entry = entries_[(sample.timestamp / BucketSeconds) % PointCount];
        if (entry.timestamp > sample.timestamp)
            return false;
        entry = sample;
        return true;
    }

    // Always order from 24 hours ago (left) to now (right). Empty buckets stay
    // empty, and the exact timestamps exclude expired or future readings.
    bool read(uint32_t now, Samples& samples, Validity& valid) const
    {
        samples = {};
        valid.fill(false);
        if (!now)
            return false;

        bool any = false;
        const uint32_t currentBucket = now / BucketSeconds;
        for (const auto& entry : entries_)
        {
            if (!entry.timestamp || entry.timestamp > now || now - entry.timestamp > WindowSeconds)
                continue;
            const uint32_t ageBuckets = currentBucket - entry.timestamp / BucketSeconds;
            if (ageBuckets >= PointCount)
                continue;
            const size_t index = PointCount - 1 - ageBuckets;
            samples[index] = entry;
            valid[index] = true;
            any = true;
        }
        return any;
    }

private:
    Samples entries_{};
};
