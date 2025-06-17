#pragma once

#include "i2c_bus.h"
#include <ctime>

static uint8_t get_day_of_week(const uint32_t year, uint32_t month, uint32_t day)
{
    uint32_t a = month < 3 ? 1 : 0;
    uint32_t b = year - a;

#if LV_CALENDAR_WEEK_STARTS_MONDAY
    uint32_t day_of_week =
        (day + (31 * (month - 2 + 12 * a) / 12) + b + (b / 4) - (b / 100) + (b / 400) - 1) % 7;
#else
    uint32_t day_of_week =
        (day + (31 * (month - 2 + 12 * a) / 12) + b + (b / 4) - (b / 100) + (b / 400)) % 7;
#endif

    return day_of_week;
}

class BM8563
{
public:
    struct Time_t {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
    };

    struct Date_t {
        uint8_t  day;
        uint8_t  weekDay;
        uint8_t  month;
        uint16_t year;
    };

    static void init(i2c_bus_handle_t bus);

    static BM8563& instance();

    BM8563(const BM8563&)            = delete;
    BM8563& operator=(const BM8563&) = delete;

    bool isVoltageLow() const;

    Time_t getTime() const;
    Date_t getDate() const;

    void setTime(Time_t time) const;
    void setDate(Date_t date) const;

    time_t getUnixTimeStamp() const;
    void   setUnixTimeStamp(uint32_t timestamp);

private:
    explicit BM8563(i2c_bus_handle_t bus);
    ~BM8563();

    i2c_bus_device_handle_t i2c_device_;
    static inline BM8563*   instance_ = nullptr;
    bool                    is_need_update;
};