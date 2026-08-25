#pragma once

#include "adapters/lvgl/lvgl_port_v8.h"
#include <cstdint>

class Brightness
{
    static constexpr uint8_t DefaultValueWhenNotConfigured = 100;
    static constexpr const char*   TAG                           = "Brightness";

    // Auto brightness schedule (local time): full brightness during the day,
    // dimmed in the evening, lowest at night.
    static constexpr uint8_t DayBrightnessPercent     = 100;
    static constexpr uint8_t EveningBrightnessPercent = 60;
    static constexpr uint8_t NightBrightnessPercent   = 20;
    static constexpr int     DayStartHour             = 6;
    static constexpr int     EveningStartHour         = 18;
    static constexpr int     NightStartHour           = 22;

    // How often the auto brightness schedule is re-evaluated.
    static constexpr uint32_t AutoUpdatePeriodMs = 60 * 60 * 1000;

    uint8_t                  level_;
    bool                     autoUpdate_;
    lv_obj_t*                mask;
    lv_timer_t*              brightnessTimer;
    void                     createMask();
    void                     setMaskOpacity(uint8_t);
    Brightness(); // Constructor

public:
    static Brightness& instance()
    {
        static Brightness instance;
        return instance;
    }
    bool init();

    bool set(bool, uint8_t);
    bool get(bool*, uint8_t*);
    bool update();
};
