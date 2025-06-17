#pragma once

#include "ui/Dashboard/Dashboard.h"

static void set_system_time(void* pv)
{
    const TickType_t xDelay        = pdMS_TO_TICKS(30000);
    TickType_t       xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        if (*reinterpret_cast<uint8_t*>(pv))
            break;

        lvgl_port_lock(-1);
        time_t time_val = BM8563::instance().getUnixTimeStamp();
        auto   t        = static_cast<uint32_t>(time_val);
        ESP_LOGD("timestamp", "set_system_time: time=%lu", t);
        Dashboard::instance().updateTimeLabel(t, 3 * 3600);
        lvgl_port_unlock();
        vTaskDelayUntil(&xLastWakeTime, xDelay);
    }
    vTaskDelete(nullptr);
}

class TimeStamp
{
    TimeStamp() = default;

public:
    uint8_t is_sync_current_time = 0;

    static TimeStamp& instance()
    {
        static TimeStamp instance;
        return instance;
    }
    TimeStamp& operator=(const TimeStamp&) = delete;
    TimeStamp(const TimeStamp&)            = delete;

    void init()
    {
        xTaskCreateWithCaps(set_system_time, "set_system_time", 4096, &is_sync_current_time, 1,
                            nullptr, MALLOC_CAP_SPIRAM);
    }
};