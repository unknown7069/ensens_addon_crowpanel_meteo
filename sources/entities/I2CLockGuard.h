#pragma once

#include "adapters/lvgl/lvgl_port_v8.h"

class I2CLockGuard
{
public:
    I2CLockGuard()
    {
        lvgl_port_lock(-1);
    }

    ~I2CLockGuard()
    {
        lvgl_port_unlock();
    }
};
