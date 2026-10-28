#pragma once

#include "esp_heap_caps.h"
#include "lvgl.h"

struct StatusMetricWidgets {
    lv_obj_t* value;
    lv_obj_t* chart;
    lv_chart_series_t* series;
};

// Widget handles of the dashboard view and the six sensor status rows.
struct DashboardWidgets {
    lv_obj_t* parent;
    lv_obj_t* cont;
    lv_obj_t* tab_dashboard;
    lv_obj_t* tab_settings;
    lv_obj_t* tab_status;
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
    StatusMetricWidgets status_metrics[6];
    int32_t   tab_h;
};

// Builds the dashboard tabs (dashboard, status, settings placeholder) under
// "parent" and returns the allocated widget bundle.
DashboardWidgets* dashboard_view_create(lv_obj_t* parent, int32_t tab_h);
