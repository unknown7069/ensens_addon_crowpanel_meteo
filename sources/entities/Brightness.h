#pragma once

#include "adapters/lvgl/lvgl_port_v8.h"
#include <cstdint>

class Brightness
{
    static constexpr uint8_t DefaultValueWhenNotConfigured = 100;
    static constexpr const char*   Tag                           = "Brightness";
    uint8_t                  _level;
    bool                     _autoUpdate;
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
