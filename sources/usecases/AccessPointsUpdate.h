#pragma once

#include "freertos/FreeRTOS.h"
#include "entities/ui/wifi_screen/WifiScreen.h"
#include "entities/ui/wifi_screen/elements/AccessPointItem.h"

namespace UseCases
{
class AccessPointsUpdate
{
    static constexpr int      TaskSize       = 4096;
    static constexpr int      TaskPriority   = 0;
    static constexpr const char*    TaskName       = "APUpd";
    static constexpr const char*    TAG            = "AccessPointUpdate";
    static constexpr uint32_t UpdatePeriodMs = 60000;

    TaskHandle_t                  taskHandle_ = nullptr;
    static void                   task(void*);
    std::vector<AccessPointItem*> wifiList_;

public:
    static AccessPointsUpdate& instance()
    {
        static AccessPointsUpdate instance;
        return instance;
    }
    bool init();
};
} // namespace UseCases