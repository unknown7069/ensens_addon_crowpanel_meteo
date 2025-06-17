#include "driver/i2c.h"
#include "esp_log.h"

#include "BM8563.h"
#include "I2CLockGuard.h"

#include <stdexcept>

static auto TAG = "BM8563";

#define BM8563_I2C_DEFAULT_ADDRESS 0x51

// BM8563 contains 16 registers with an auto-incrementing address register
#define BM8563_REG_CONTROL_STATUS1 0x00
#define BM8563_REG_CONTROL_STATUS2 0x01
#define BM8563_REG_SECONDS 0x02
#define BM8563_REG_MINUTES 0x03
#define BM8563_REG_HOURS 0x04
#define BM8563_REG_DAYS 0x05
#define BM8563_REG_WEEKDAYS 0x06
#define BM8563_REG_MONTHS_CENTURY 0x07
#define BM8563_REG_YEARS 0x08
#define BM8563_REG_MINUTE_ALARM 0x09
#define BM8563_REG_HOUR_ALARM 0x0a
#define BM8563_REG_DATE_ALARM 0x0b
#define BM8563_REG_WEEKDAY_ALARM 0x0c
#define BM8563_REG_CLKOUT_CONTROL 0x0d
#define BM8563_REG_TIMER_CONTROL 0x0e
#define BM8563_REG_TIMER_COUNTDOWN 0x0f

#define BM8563_VL_MSK 0x80
#define BM8563_SECONDS_MSK 0x7f
#define BM8563_MINUTES_MSK 0x7f
#define BM8563_HOURS_MSK 0x3f
#define BM8563_DAYS_MSK 0x3f
#define BM8563_WEEKDAYS_MSK 0x07
#define BM8563_CENTURY_MSK 0x80
#define BM8563_MONTHS_MSK 0x1f
#define BM8563_YEARS_MSK 0xff

static uint8_t BCDToByte(const uint8_t value)
{
    uint8_t tmp = 0;
    tmp =
        (static_cast<uint8_t>(value & static_cast<uint8_t>(0xF0)) >> static_cast<uint8_t>(4)) * 10;
    return tmp + (value & static_cast<uint8_t>(0x0F));
}

static uint8_t ByteToBCD(uint8_t value)
{
    uint8_t bcdhigh = 0;
    while (value >= 10)
    {
        bcdhigh++;
        value -= 10;
    }
    return static_cast<uint8_t>(bcdhigh << 4) | value;
}

BM8563::BM8563(i2c_bus_handle_t bus)
{
    I2CLockGuard guard;

    i2c_device_ = i2c_bus_device_create(bus, BM8563_I2C_DEFAULT_ADDRESS, 0);
    // clear VL bit
    uint8_t sec;
    i2c_bus_read_bytes(i2c_device_, BM8563_REG_SECONDS, 1, &sec);
    sec &= ~BM8563_VL_MSK;
    i2c_bus_write_byte(i2c_device_, BM8563_REG_SECONDS, sec);

    // clear STOP bit
    uint8_t ctrl1;
    i2c_bus_read_bytes(i2c_device_, BM8563_REG_CONTROL_STATUS1, 1, &ctrl1);
    ctrl1 &= ~(1 << 5);
    i2c_bus_write_byte(i2c_device_, BM8563_REG_CONTROL_STATUS1, ctrl1);

    is_need_update = true;
}

BM8563::~BM8563()
{
    I2CLockGuard guard;

    i2c_bus_device_delete(&i2c_device_);
    delete instance_;
    instance_ = nullptr;
}

void BM8563::init(i2c_bus_handle_t bus)
{
    if (!instance_)
    {
        instance_ = new BM8563(bus);
    }
}

BM8563& BM8563::instance()
{
    if (!instance_)
    {
        throw std::runtime_error("BM8563 not initialized!");
    }
    return *instance_;
}

BM8563::Date_t BM8563::getDate() const
{
    I2CLockGuard guard;

    uint8_t buf[4];
    i2c_bus_read_bytes(i2c_device_, BM8563_REG_DAYS, sizeof(buf), buf);
    ESP_LOG_BUFFER_HEXDUMP(TAG, buf, sizeof(buf), ESP_LOG_VERBOSE);
    Date_t date;
    date.day     = BCDToByte(buf[0] & BM8563_DAYS_MSK);
    date.weekDay = BCDToByte(buf[1] & BM8563_WEEKDAYS_MSK);
    date.month   = BCDToByte(buf[2] & BM8563_MONTHS_MSK);
    date.year    = BCDToByte(buf[3] & BM8563_YEARS_MSK);
    if (buf[2] & BM8563_CENTURY_MSK)
    {
        date.year += 1900;
    } else
    {
        date.year += 2000;
    }
    return date;
}

BM8563::Time_t BM8563::getTime() const
{
    I2CLockGuard guard;

    uint8_t buf[3];
    i2c_bus_read_bytes(i2c_device_, BM8563_REG_SECONDS, sizeof(buf), buf);
    ESP_LOG_BUFFER_HEXDUMP(TAG, buf, sizeof(buf), ESP_LOG_VERBOSE);
    Time_t time;
    time.seconds = BCDToByte(buf[0] & BM8563_SECONDS_MSK);
    time.minutes = BCDToByte(buf[1] & BM8563_MINUTES_MSK);
    time.hours   = BCDToByte(buf[2] & BM8563_HOURS_MSK);
    if (buf[0] & BM8563_VL_MSK)
    {
        ESP_LOGW(TAG, "voltage-low detected, need to reset time");
    }
    return time;
}

bool BM8563::isVoltageLow() const
{
    I2CLockGuard guard;

    uint8_t buf[1];
    i2c_bus_read_bytes(i2c_device_, BM8563_REG_SECONDS, sizeof(buf), buf);
    return buf[0] & BM8563_VL_MSK;
}

void BM8563::setDate(const Date_t date) const
{
    I2CLockGuard guard;

    i2c_bus_write_byte(i2c_device_, BM8563_REG_DAYS, ByteToBCD(date.day) & BM8563_DAYS_MSK);
    i2c_bus_write_byte(i2c_device_, BM8563_REG_WEEKDAYS,
                       ByteToBCD(date.weekDay) & BM8563_WEEKDAYS_MSK);
    uint8_t months_century_val = ByteToBCD(date.month) & BM8563_MONTHS_MSK;
    if (date.year < 2000)
    {
        months_century_val |= BM8563_CENTURY_MSK;
    }
    i2c_bus_write_byte(i2c_device_, BM8563_REG_MONTHS_CENTURY, months_century_val);
    i2c_bus_write_byte(i2c_device_, BM8563_REG_YEARS,
                       ByteToBCD(static_cast<uint8_t>(date.year % 100)) & BM8563_YEARS_MSK);
}

void BM8563::setTime(const Time_t time) const
{
    I2CLockGuard guard;

    uint8_t time_buf[3];
    time_buf[0] = static_cast<uint8_t>(ByteToBCD(time.seconds) & BM8563_SECONDS_MSK);
    time_buf[1] = static_cast<uint8_t>(ByteToBCD(time.minutes) & BM8563_MINUTES_MSK);
    time_buf[2] = static_cast<uint8_t>(ByteToBCD(time.hours) & BM8563_HOURS_MSK);
    i2c_bus_write_bytes(i2c_device_, BM8563_REG_SECONDS, sizeof(time_buf), time_buf);
}

time_t BM8563::getUnixTimeStamp() const
{
    I2CLockGuard guard;

    BM8563::Time_t rtc_time = getTime();
    BM8563::Date_t rtc_date = getDate();

    struct tm time_struct = {};
    time_struct.tm_year   = rtc_date.year - 1900; /* Year - 1900 */
    time_struct.tm_mon    = rtc_date.month - 1;   /* Month (0-11) */
    time_struct.tm_mday   = rtc_date.day;         /* Day of the month (1-31) */
    time_struct.tm_wday   = rtc_date.weekDay;     /* Day of the week (0-6, Sunday = 0) */
    time_struct.tm_hour   = rtc_time.hours;       /* Hours (0-23) */
    time_struct.tm_min    = rtc_time.minutes;     /* Minutes (0-59) */
    time_struct.tm_sec    = rtc_time.seconds;     /* Seconds (0-60) */

    time_t timestamp = mktime(&time_struct);
    if (timestamp == -1)
    {
        ESP_LOGE(TAG, "mktime failed");
    }
    return timestamp;
}

void BM8563::setUnixTimeStamp(uint32_t timestamp)
{
    I2CLockGuard guard;

    if (timestamp != 0 && is_need_update)
    {
        time_t     timestampStruct = timestamp;
        struct tm* timeInfo        = localtime(&timestampStruct);

        Time_t time_ = { (uint8_t)timeInfo->tm_hour, (uint8_t)timeInfo->tm_min,
                         (uint8_t)timeInfo->tm_sec };

        uint16_t year_ = (uint16_t)(timeInfo->tm_year + 1900); /* Year - 1900 */
        uint8_t  mon_  = (uint8_t)(timeInfo->tm_mon + 1);      /* Month (0-11) */

        Date_t date_ = { (uint8_t)timeInfo->tm_mday, (uint8_t)timeInfo->tm_wday, mon_, year_ };

        setTime(time_);
        setDate(date_);
        is_need_update = false;
    }
}
