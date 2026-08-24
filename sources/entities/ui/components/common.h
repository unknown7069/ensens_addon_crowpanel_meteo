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

typedef struct {
    lv_obj_t* parent;
    lv_obj_t* cont;
    lv_obj_t* tab_dashboard;
    lv_obj_t* tab_settings;
    lv_obj_t* tab_history;
    lv_obj_t* tab_bar;
    lv_obj_t* label;
    lv_obj_t* temp_inside_label;
    lv_obj_t* temp_outside_label;
    lv_obj_t* humidity_inside_label;
    lv_obj_t* humidity_outside_label;
    lv_obj_t* pressure_inside_label;
    lv_obj_t* pressure_outside_label;
    lv_obj_t* wind_speed_label;
    lv_obj_t* feels_like_label;
    lv_obj_t* daily_high_label;
    lv_obj_t* daily_low_label;
    lv_obj_t* precipitation_outside_label;
    lv_obj_t* voc_label;
    lv_obj_t* co2_label;
    lv_obj_t* iaq_label;
    lv_obj_t* date_label;
    lv_obj_t* bottom_plot_title;
    lv_obj_t* bottom_plot_chart;
    lv_chart_series_t* bottom_plot_series;
    lv_chart_cursor_t* bottom_plot_cursor;
    int32_t   tab_h;
} tabview_t;

extern lv_style_t transparent_area_style;

/**
 * @brief  Init common styles
 */
void ui_styles_init();

tabview_t* tabview_create(lv_obj_t* parent, int32_t tab_h);

#endif
