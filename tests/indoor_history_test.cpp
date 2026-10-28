#include "entities/IndoorMetricHistory.h"

#include <algorithm>
#include <cassert>

using History = IndoorMetricHistory;
constexpr uint32_t Hour = 3600;
constexpr uint32_t Midnight = 20000U * 86400;

History::Sample sample(uint32_t timestamp, float value, UnitType unit = Celsius)
{
    History::Sample reading{};
    reading.timestamp = timestamp;
    reading.value = value;
    reading.unit = unit;
    return reading;
}

int main()
{
    History::Samples values{};
    History::Validity valid{};
    History empty;
    valid.fill(true);
    assert(!empty.read(Midnight, values, valid));
    assert(std::none_of(valid.begin(), valid.end(), [](bool v) { return v; }));
    assert(!empty.store(sample(0, 10)));

    // Crossing midnight retains the previous evening, ordered before today.
    History midnight;
    assert(midnight.store(sample(Midnight - 15 * 60, 20)));
    assert(midnight.store(sample(Midnight + 5 * 60, 21)));
    assert(midnight.read(Midnight + 10 * 60, values, valid));
    assert(valid[History::PointCount - 2] && valid.back());
    assert(values[History::PointCount - 2].value == 20 && values.back().value == 21);

    // Each bucket keeps its latest reading and original unit metadata.
    assert(midnight.store(sample(Midnight + 9 * 60, 71, Fahrenheit)));
    assert(!midnight.store(sample(Midnight + 6 * 60, -99)));
    assert(midnight.read(Midnight + 10 * 60, values, valid));
    assert(values.back().value == 71 && values.back().unit == Fahrenheit);

    // Exactly 24h of half-hour points survive wraparound, including both ends.
    History full;
    for (size_t i = 0; i < History::PointCount; ++i)
        assert(full.store(sample(Midnight - History::WindowSeconds + i * History::BucketSeconds, i)));
    assert(full.read(Midnight, values, valid));
    for (size_t i = 0; i < values.size(); ++i) {
        assert(valid[i] && values[i].value == i);
        if (i) assert(values[i - 1].timestamp < values[i].timestamp);
    }
    assert(full.store(sample(Midnight + History::BucketSeconds, History::PointCount)));
    assert(full.read(Midnight + History::BucketSeconds, values, valid));
    for (size_t i = 0; i < values.size(); ++i)
        assert(valid[i] && values[i].value == i + 1);

    // A late old packet cannot overwrite a newer entry at the same ring index.
    assert(!full.store(sample(Midnight - History::WindowSeconds, -99)));
    assert(full.read(Midnight + History::BucketSeconds, values, valid));
    assert(values.back().value == History::PointCount);

    // Both partial end buckets must fit. Expiry uses exact age, not slot age.
    History partial;
    const uint32_t now = Midnight + 10 * 60;
    assert(partial.store(sample(now - History::WindowSeconds + 60, -2)));
    assert(partial.store(sample(now, 3)));
    assert(partial.read(now, values, valid));
    assert(valid.front() && valid.back());
    assert(values.front().value == -2 && values.back().value == 3);
    assert(partial.read(now + 120, values, valid));
    assert(!valid.front() && valid.back());

    // Missing readings stay as gaps, with no invented carry-forward values.
    History sparse;
    assert(sparse.store(sample(now - 10 * Hour, 12)));
    assert(sparse.store(sample(now, 14)));
    assert(sparse.read(now, values, valid));
    assert(std::count(valid.begin(), valid.end(), true) == 2);
    // A clock query alone ages readings off when the sensor goes offline.
    assert(!sparse.read(now + History::WindowSeconds + 1, values, valid));
    assert(std::none_of(valid.begin(), valid.end(), [](bool v) { return v; }));
    // A clock query before a recorded reading must not plot future data.
    assert(sparse.read(now - 1, values, valid));
    assert(std::count(valid.begin(), valid.end(), true) == 1);
    assert(sparse.store(sample(now + 10 * History::WindowSeconds, 15)));
    assert(sparse.read(now + 10 * History::WindowSeconds, values, valid));
    assert(std::count(valid.begin(), valid.end(), true) == 1 && values.back().value == 15);
    assert(!sparse.read(0, values, valid));
    assert(std::none_of(valid.begin(), valid.end(), [](bool v) { return v; }));
}
