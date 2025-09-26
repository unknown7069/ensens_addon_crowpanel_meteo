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

#define CUR_DAY_SYMBOL "-"
// Need to take into account one digit after the decimal point
#define MULT_COEFF (uint16_t)10
LV_IMG_DECLARE(humidity_icon);
LV_FONT_DECLARE(Montserrat_96);

typedef struct {
    lv_obj_t* cont;
    lv_obj_t* int_part;
    lv_obj_t* frac_part;
    lv_obj_t* unit_part;
} number_value_t;

typedef struct {
    lv_obj_t*             cont;
    lv_obj_t*             meter;
    lv_meter_scale_t*     scale;
    lv_meter_indicator_t* needle;
} meter_t;

typedef struct {
    lv_obj_t*          cont;
    lv_obj_t*          chart;
    lv_chart_series_t* y_prim;
    lv_chart_series_t* y_sec;
} plot_t;

typedef struct {
    lv_obj_t*       cont;
    lv_obj_t*       title;
    lv_obj_t*       dimension;
    meter_t*        gauge;
    meter_t*        tend;
    number_value_t* value;
} default_box_t;

typedef struct {
    lv_obj_t*       cont;
    meter_t*        tend;
    number_value_t* value;
} data_box_t;

typedef struct {
    lv_obj_t*       cont;
    lv_obj_t*       title_dew;
    meter_t*        tend;
    number_value_t* value_humi;
    number_value_t* value_dew;
    lv_obj_t*       humiIcon;
} humi_dew_data_box_t;

typedef struct {
    lv_obj_t* cont;
    lv_obj_t* current_day[7];
    lv_obj_t* days_labels[7];
    lv_obj_t* zodizk_sign;
} days_header_t;

typedef struct {
    lv_obj_t* parent;
    lv_obj_t* cont;
    lv_obj_t* tab_1;
    lv_obj_t* tab_2;
    lv_obj_t* tab_3;
    lv_obj_t* tab_bar;
    lv_obj_t* label;
    int32_t   tab_h;
} tabview_t;

extern lv_style_t transparent_area_style;

/**
 * @brief  Init common styles
 */
void ui_styles_init();

number_value_t*      number_value_create(lv_obj_t* parent);
meter_t*             tendency_meter_create(lv_obj_t* parent);
default_box_t*       default_box_create(lv_obj_t* parent);
data_box_t*          data_box_create(lv_obj_t* parent);
humi_dew_data_box_t* humi_dew_box_create(lv_obj_t* parent);
tabview_t*           tabview_create(lv_obj_t* parent, int32_t tab_h);
days_header_t*       days_header_create(lv_obj_t* parent);

lv_obj_t* draw_triangle(lv_obj_t* parent, lv_coord_t width, lv_coord_t height);
lv_obj_t* draw_filled_half_circle(lv_obj_t* parent, lv_coord_t radius, lv_color_t color);
lv_obj_t* draw_circle(lv_obj_t* parent, lv_coord_t radius);

#endif
