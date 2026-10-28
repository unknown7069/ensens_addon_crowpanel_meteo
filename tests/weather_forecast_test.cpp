#include "entities/WeatherForecast.h"
#include <cassert>

int main()
{
    constexpr int64_t midnight = 20000LL * 86400;
    constexpr int64_t hour = 3600;
    WeatherForecast evening(midnight + 20 * hour);
    evening.add(midnight + 17 * hour, -99); // Previous daytime is excluded.
    evening.add(midnight + 21 * hour, -4);
    evening.add(midnight + 27 * hour, -9);
    evening.add(midnight + 30 * hour, 2); // 06:00 belongs to the day.
    evening.add(midnight + 39 * hour, 12);
    evening.add(midnight + 42 * hour, 99); // Next night is excluded.
    assert(evening.nightLow == -9 && evening.dayHigh == 12);

    WeatherForecast morning(midnight + 10 * hour);
    assert(morning.nightStart == evening.nightStart);
    assert(morning.dayStart == midnight + 30 * hour);

    WeatherForecast afterMidnight(midnight + 3 * hour);
    afterMidnight.add(midnight + 4 * hour, -15);
    afterMidnight.add(midnight + 6 * hour, -3);
    afterMidnight.add(midnight + 15 * hour, 7);
    assert(afterMidnight.nightLow == -15 && afterMidnight.dayHigh == 7);

    WeatherForecast dawn(midnight + 6 * hour);
    assert(dawn.nightStart == midnight + 18 * hour);
    assert(std::isnan(dawn.nightLow) && std::isnan(dawn.dayHigh));
    dawn.add(dawn.nightStart, std::numeric_limits<float>::quiet_NaN());
    assert(std::isnan(dawn.nightLow));

    // UTC 02:00 with a -06:00 location offset is the previous local evening.
    WeatherForecast west(midnight + 2 * hour - 6 * hour);
    assert(west.nightStart == midnight - 6 * hour);
    assert(west.dayStart == midnight + 6 * hour);

    // Exactly eight upcoming three-hour points, with no stale or >24h data.
    WeatherForecast timeline(midnight + 8 * hour + 42 * 60);
    timeline.add(midnight + 6 * hour, -99, "01n");
    for (int i = 0; i < 24; ++i)
        timeline.add(midnight + (9 + i * 3) * hour, 6.f - i, "02d");
    assert(timeline.count == WeatherForecast::Capacity);
    auto next = timeline.next24Hours(timeline.referenceTime);
    assert(next.front().timestamp == midnight + 9 * hour);
    assert(next.back().timestamp == midnight + 30 * hour);
    assert(timeline.next24Hours(midnight + 9 * hour).front().timestamp == midnight + 12 * hour);
    assert(timeline.next24Hours(midnight + 100 * hour).front().timestamp == 0);

    WeatherForecast sparse(midnight + 8 * hour);
    sparse.add(midnight + 9 * hour, 3, "01d");
    sparse.add(midnight + 9 * hour, -100, "13d"); // Duplicate cannot change a slot.
    sparse.add(midnight + 12 * hour, std::numeric_limits<float>::quiet_NaN());
    sparse.add(midnight + 33 * hour, 9, "01d"); // Outside the 24-hour window.
    next = sparse.next24Hours(midnight + 8 * hour);
    assert(next[0].temperature == 3 && next[1].timestamp == 0);
}
