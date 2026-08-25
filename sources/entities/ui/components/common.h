/* Common header

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include "esp_heap_caps.h"
#include "lvgl.h"

LV_IMG_DECLARE(humidity_icon);
LV_IMG_DECLARE(pressure_icon);
LV_IMG_DECLARE(temperature);
LV_IMG_DECLARE(wind_icon);

LV_IMG_DECLARE(co2);
LV_IMG_DECLARE(aqi);
LV_FONT_DECLARE(saira_condensed_medium);

extern lv_style_t transparent_area_style;

/**
 * @brief  Init common styles
 */
void ui_styles_init();

#endif
