#pragma once

#include "lvgl.h"

// Resolution-independent, antialiased LVGL primitives; no image buffers.
lv_obj_t* weather_icon_create(lv_obj_t* parent, lv_coord_t size, uint32_t background);
void weather_icon_set(lv_obj_t* icon, const char* openWeatherCode);
