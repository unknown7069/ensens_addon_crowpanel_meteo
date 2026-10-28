#pragma once

#include "esp_heap_caps.h"
#include "lvgl.h"
#include "entities/WeatherForecast.h"

struct StatusMetricWidgets {
    lv_obj_t* value;
    lv_obj_t* chart;
    lv_chart_series_t* series;
};

struct ForecastPointWidgets {
    lv_obj_t* time;
    lv_obj_t* day;
    lv_obj_t* icon;
    lv_obj_t* temperature;
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
    lv_obj_t* am_pm_label;
    lv_obj_t* indoor_humidity_label;
    lv_obj_t* outside_icon;
    lv_obj_t* wifi_label;
    lv_obj_t* settings_button;
    lv_obj_t* indoor_button;
    lv_obj_t* status_back_button;
    lv_obj_t* forecast_summary_label;
    lv_obj_t* forecast_heading_label;
    lv_obj_t* forecast_band;
    lv_obj_t* forecast_graph;
    lv_obj_t* midnight_divider;
    ForecastPointWidgets forecast_points[WeatherForecast::DisplayPoints];
    // Retained draw state, avoiding allocations when the clock ticks.
    lv_point_t forecast_curve[WeatherForecast::DisplayPoints];
    bool forecast_valid[WeatherForecast::DisplayPoints];
    StatusMetricWidgets status_metrics[6];
    int32_t   tab_h;
};

// Builds the dashboard tabs (dashboard, status, settings placeholder) under
// "parent" and returns the allocated widget bundle.
DashboardWidgets* dashboard_view_create(lv_obj_t* parent, int32_t tab_h);
void dashboard_view_update_forecast(DashboardWidgets* widgets, const WeatherForecast& forecast,
                                     int64_t localNow, bool fahrenheit);
void dashboard_view_update_clock(DashboardWidgets* widgets, int64_t localTimestamp);
