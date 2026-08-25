#include "timestamp.h"

#include "adapters/lvgl/lvgl_port_v8.h"
#include "entities/BM8563.h"
#include "entities/CurrentTime.h"
#include "entities/ui/Dashboard/Dashboard.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static constexpr char TAG[] = "rtc-clock";
static constexpr uint32_t PublishPeriodMs = 60 * 1000;

static void publishRtcTimeTask(void* pv)
{
    (void)pv;
    const TickType_t updatePeriod  = pdMS_TO_TICKS(PublishPeriodMs);
    TickType_t       xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        lvgl_port_lock(-1);
        time_t time_val = BM8563::instance().getUnixTimeStamp();
        ESP_LOGD(TAG, "set_system_time: time=%lu", static_cast<unsigned long>(time_val));
        Dashboard::instance().updateTimeLabel(static_cast<uint32_t>(time_val),
                                              CurrentTime::instance().getTimezoneOffset());
        lvgl_port_unlock();
        vTaskDelayUntil(&xLastWakeTime, updatePeriod);
    }
}

void RtcClockUpdater::init()
{
    xTaskCreateWithCaps(publishRtcTimeTask, "rtc_clock_update", 4096, nullptr, 1, nullptr,
                        MALLOC_CAP_SPIRAM);
}
