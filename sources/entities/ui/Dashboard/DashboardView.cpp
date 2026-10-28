#include "DashboardWidgets.h"
#include "KitchenTheme.h"
#include "WeatherIcon.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>

LV_FONT_DECLARE(kitchen_clock_188);
LV_FONT_DECLARE(kitchen_clock_160);
LV_FONT_DECLARE(kitchen_temperature_88);

namespace {
using namespace KitchenTheme;

lv_obj_t* label(lv_obj_t* parent, const char* text, const lv_font_t* font,
                int x, int y, int width = 0, lv_text_align_t align = LV_TEXT_ALIGN_LEFT)
{
    lv_obj_t* obj = lv_label_create(parent);
    lv_obj_set_style_text_font(obj, font, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(Ink), 0);
    lv_obj_set_style_text_align(obj, align, 0);
    if (width) {
        lv_obj_set_width(obj, width);
        lv_label_set_long_mode(obj, LV_LABEL_LONG_CLIP);
    }
    lv_label_set_text(obj, text);
    lv_obj_set_pos(obj, x, y);
    return obj;
}

void flat(lv_obj_t* obj, uint32_t color)
{
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(obj, lv_color_hex(Ink), 0);
}

void navigate(lv_event_t* e)
{
    auto* w = static_cast<DashboardWidgets*>(lv_event_get_user_data(e));
    lv_obj_t* target = lv_event_get_current_target(e);
    uint32_t index = target == w->settings_button ? 2 :
                     target == w->indoor_button ? 1 : 0;
    lv_tabview_set_act(w->cont, index, LV_ANIM_OFF);
}

lv_obj_t* textButton(lv_obj_t* parent, const char* text, int x, int y,
                     int width, DashboardWidgets* w)
{
    lv_obj_t* button = lv_btn_create(parent);
    flat(button, Paper);
    lv_obj_set_size(button, width, 48);
    lv_obj_set_pos(button, x, y);
    lv_obj_set_style_bg_color(button, lv_color_hex(Oatmeal), LV_STATE_PRESSED);
    lv_obj_set_style_radius(button, 8, 0);
    auto* caption = label(button, text, &lv_font_montserrat_24, 0, 0);
    lv_obj_center(caption);
    lv_obj_add_event_cb(button, navigate, LV_EVENT_CLICKED, w);
    return button;
}

void drawForecast(lv_event_t* e)
{
    auto* w = static_cast<DashboardWidgets*>(lv_event_get_user_data(e));
    lv_area_t bounds;
    lv_obj_get_coords(w->forecast_graph, &bounds);
    auto* ctx = lv_event_get_draw_ctx(e);
    lv_draw_line_dsc_t line;
    lv_draw_line_dsc_init(&line);
    line.color = lv_color_hex(Terracotta);
    line.width = 2;
    line.round_start = line.round_end = true;
    lv_draw_rect_dsc_t dot;
    lv_draw_rect_dsc_init(&dot);
    dot.bg_color = line.color;
    dot.radius = LV_RADIUS_CIRCLE;
    for (size_t i = 0; i < WeatherForecast::DisplayPoints; ++i) {
        if (!w->forecast_valid[i]) continue;
        lv_point_t point = {static_cast<lv_coord_t>(bounds.x1 + w->forecast_curve[i].x),
                            static_cast<lv_coord_t>(bounds.y1 + w->forecast_curve[i].y)};
        if (i && w->forecast_valid[i - 1]) {
            lv_point_t previous = {
                static_cast<lv_coord_t>(bounds.x1 + w->forecast_curve[i - 1].x),
                static_cast<lv_coord_t>(bounds.y1 + w->forecast_curve[i - 1].y)};
            lv_draw_line(ctx, &line, &previous, &point);
        }
        lv_area_t area = {static_cast<lv_coord_t>(point.x - 3), static_cast<lv_coord_t>(point.y - 3),
                          static_cast<lv_coord_t>(point.x + 3), static_cast<lv_coord_t>(point.y + 3)};
        lv_draw_rect(ctx, &dot, &area);
    }
}

void initStatus(DashboardWidgets* w)
{
    w->tab_status = lv_tabview_add_tab(w->cont, "Status");
    flat(w->tab_status, Paper);
    lv_obj_clear_flag(w->tab_status, LV_OBJ_FLAG_SCROLLABLE);
    w->status_back_button = textButton(w->tab_status, LV_SYMBOL_LEFT "  Home", 28, 16, 120, w);
    label(w->tab_status, "Indoor conditions", &lv_font_montserrat_28, 176, 17);
    auto* subtitle = label(w->tab_status, "Today's history / 00:00 - 24:00",
                            &lv_font_montserrat_16, 176, 53);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(Muted), 0);

    static const char* titles[] = { "Temperature", "Humidity", "Pressure", "CO2", "VOC", "Air quality" };
    for (size_t i = 0; i < 6; ++i) {
        lv_obj_t* card = lv_obj_create(w->tab_status);
        flat(card, Oatmeal);
        lv_obj_set_style_radius(card, 12, 0);
        lv_obj_set_pos(card, 28 + (i % 3) * 254, 94 + (i / 3) * 192);
        lv_obj_set_size(card, 236, 176);
        lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
        auto* title = label(card, titles[i], &lv_font_montserrat_18, 18, 16);
        lv_obj_set_style_text_color(title, lv_color_hex(Muted), 0);
        auto& metric = w->status_metrics[i];
        metric.value = label(card, "--", &lv_font_montserrat_30, 18, 48, 200);
        metric.chart = lv_chart_create(card);
        lv_obj_set_pos(metric.chart, 18, 112);
        lv_obj_set_size(metric.chart, 200, 48);
        lv_obj_clear_flag(metric.chart, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(metric.chart, 2, 0);
        lv_obj_set_style_bg_opa(metric.chart, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(metric.chart, 0, 0);
        lv_obj_set_style_size(metric.chart, 0, LV_PART_INDICATOR);
        lv_chart_set_type(metric.chart, LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(metric.chart, 48);
        lv_chart_set_div_line_count(metric.chart, 2, 0);
        metric.series = lv_chart_add_series(metric.chart, lv_color_hex(Terracotta), LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_all_value(metric.chart, metric.series, LV_CHART_POINT_NONE);
    }
}

void initDashboard(DashboardWidgets* w)
{
    w->tab_dashboard = lv_tabview_add_tab(w->cont, "Clock");
    lv_obj_t* home = w->tab_dashboard;
    flat(home, Paper);
    lv_obj_clear_flag(home, LV_OBJ_FLAG_SCROLLABLE);
    w->date_label = label(home, "Setting the clock...", &lv_font_montserrat_24, 28, 20, 620);
    w->wifi_label = label(home, LV_SYMBOL_WIFI, &lv_font_montserrat_24, 690, 21);
    lv_obj_set_style_text_opa(w->wifi_label, LV_OPA_40, 0);
    w->settings_button = textButton(home, LV_SYMBOL_SETTINGS, 738, 9, 48, w);

    w->label = label(home, "--:--", &kitchen_clock_188, 20, 76);
    w->am_pm_label = label(home, "", &lv_font_montserrat_28, 434, 178);

    w->indoor_button = lv_btn_create(home);
    flat(w->indoor_button, Paper);
    lv_obj_set_pos(w->indoor_button, 28, 222);
    lv_obj_set_size(w->indoor_button, 464, 44);
    lv_obj_set_flex_flow(w->indoor_button, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(w->indoor_button, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(w->indoor_button, 8, 0);
    lv_obj_set_style_bg_color(w->indoor_button, lv_color_hex(Oatmeal), LV_STATE_PRESSED);
    lv_obj_add_event_cb(w->indoor_button, navigate, LV_EVENT_CLICKED, w);
    label(w->indoor_button, "Inside", &lv_font_montserrat_24, 0, 0);
    w->temp_inside_label = label(w->indoor_button, "--°", &lv_font_montserrat_24, 0, 0);
    label(w->indoor_button, "•  Humidity", &lv_font_montserrat_24, 0, 0);
    w->indoor_humidity_label = label(w->indoor_button, "--%", &lv_font_montserrat_24, 0, 0);

    w->outside_icon = weather_icon_create(home, 120, Paper);
    lv_obj_set_pos(w->outside_icon, 513, 83);
    w->temp_outside_label = label(home, "--°", &kitchen_temperature_88, 630, 96, 144, LV_TEXT_ALIGN_RIGHT);
    label(home, "Outside now", &lv_font_montserrat_20, 631, 181, 143, LV_TEXT_ALIGN_RIGHT);
    w->feels_like_label = label(home, "Feels like --°C", &lv_font_montserrat_20, 504, 215, 270, LV_TEXT_ALIGN_RIGHT);
    lv_obj_set_style_text_color(w->feels_like_label, lv_color_hex(Muted), 0);

    w->forecast_band = lv_obj_create(home);
    flat(w->forecast_band, Oatmeal);
    lv_obj_set_pos(w->forecast_band, 0, 274);
    lv_obj_set_size(w->forecast_band, 800, 206);
    lv_obj_clear_flag(w->forecast_band, LV_OBJ_FLAG_SCROLLABLE);
    w->forecast_heading_label = label(w->forecast_band, "NEXT 24 HOURS", &lv_font_montserrat_20, 28, 16, 280);
    w->forecast_summary_label = label(w->forecast_band, "Tonight --° / Tomorrow --°",
                                     &lv_font_montserrat_18, 320, 18, 454, LV_TEXT_ALIGN_RIGHT);
    w->midnight_divider = lv_obj_create(w->forecast_band);
    flat(w->midnight_divider, Rule);
    lv_obj_set_size(w->midnight_divider, 1, 140);
    lv_obj_set_pos(w->midnight_divider, 500, 56);
    lv_obj_add_flag(w->midnight_divider, LV_OBJ_FLAG_HIDDEN);
    for (size_t i = 0; i < WeatherForecast::DisplayPoints; ++i) {
        const int x = 24 + i * 94;
        auto& point = w->forecast_points[i];
        point.time = label(w->forecast_band, "--", &lv_font_montserrat_18, x, 54, 94, LV_TEXT_ALIGN_CENTER);
        point.day = label(w->forecast_band, "", &lv_font_montserrat_12, x, 77, 94, LV_TEXT_ALIGN_CENTER);
        lv_obj_set_style_text_color(point.day, lv_color_hex(Muted), 0);
        point.icon = weather_icon_create(w->forecast_band, 48, Oatmeal);
        lv_obj_set_pos(point.icon, x + 23, 91);
        point.temperature = label(w->forecast_band, "--°", &lv_font_montserrat_30, x, 168, 94, LV_TEXT_ALIGN_CENTER);
    }
    w->forecast_graph = lv_obj_create(w->forecast_band);
    lv_obj_remove_style_all(w->forecast_graph);
    lv_obj_set_pos(w->forecast_graph, 0, 139);
    lv_obj_set_size(w->forecast_graph, 800, 29);
    lv_obj_clear_flag(w->forecast_graph, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(w->forecast_graph, drawForecast, LV_EVENT_DRAW_MAIN, w);
}
}

DashboardWidgets* dashboard_view_create(lv_obj_t* parent, int32_t tab_h)
{
    auto* w = static_cast<DashboardWidgets*>(heap_caps_calloc(1, sizeof(DashboardWidgets), MALLOC_CAP_SPIRAM));
    if (!w) return nullptr;
    w->tab_h = tab_h;
    w->parent = parent;
    w->cont = lv_tabview_create(parent, LV_DIR_TOP, 0);
    flat(w->cont, KitchenTheme::Paper);
    initDashboard(w);
    initStatus(w);
    w->humidity_inside_label = w->status_metrics[1].value;
    w->pressure_inside_label = w->status_metrics[2].value;
    w->co2_label = w->status_metrics[3].value;
    w->voc_label = w->status_metrics[4].value;
    w->iaq_label = w->status_metrics[5].value;
    w->tab_settings = lv_tabview_add_tab(w->cont, "Settings");
    lv_obj_set_style_pad_all(w->tab_settings, 0, 0);
    w->tab_bar = lv_tabview_get_tab_btns(w->cont);
    lv_obj_add_flag(w->tab_bar, LV_OBJ_FLAG_HIDDEN);
    // Navigation uses explicit buttons, leaving the full display for the clock.
    lv_obj_clear_flag(lv_tabview_get_content(w->cont), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_update_layout(w->cont);
    return w;
}

void dashboard_view_update_clock(DashboardWidgets* w, int64_t localTimestamp)
{
    if (localTimestamp <= 0) return;
    const time_t local = static_cast<time_t>(localTimestamp);
    struct tm info{};
    gmtime_r(&local, &info);
    const int hour = info.tm_hour % 12 ? info.tm_hour % 12 : 12;
    char clock[8], date[64], month[48];
    std::snprintf(clock, sizeof(clock), "%d:%02d", hour, info.tm_min);
    lv_obj_set_style_text_font(w->label, hour >= 10 ? &kitchen_clock_160 : &kitchen_clock_188, 0);
    lv_label_set_text(w->label, clock);
    lv_obj_update_layout(w->label);
    lv_obj_set_pos(w->label, 20, hour >= 10 ? 96 : 76); // Keep the digit baseline level.
    lv_obj_set_pos(w->am_pm_label, 24 + lv_obj_get_width(w->label), 181);
    lv_label_set_text(w->am_pm_label, info.tm_hour < 12 ? "AM" : "PM");
    std::strftime(month, sizeof(month), "%A, %B", &info);
    std::snprintf(date, sizeof(date), "%s %d", month, info.tm_mday);
    lv_label_set_text(w->date_label, date);
}

void dashboard_view_update_forecast(DashboardWidgets* w, const WeatherForecast& forecast,
                                     int64_t localNow, bool fahrenheit)
{
    const auto next = forecast.next24Hours(localNow);
    float minimum = 10000.f, maximum = -10000.f;
    WeatherForecast summary(localNow);
    for (size_t i = 0; i < forecast.count; ++i)
        summary.add(forecast.points[i].timestamp, forecast.points[i].temperature);
    for (const auto& point : next) {
        if (!point.timestamp) continue;
        minimum = std::min(minimum, point.temperature);
        maximum = std::max(maximum, point.temperature);
    }
    const auto temperature = [fahrenheit](float c) { return fahrenheit ? c * 1.8f + 32.f : c; };
    char low[16] = "--°", high[16] = "--°", text[96];
    if (std::isfinite(summary.nightLow))
        std::snprintf(low, sizeof(low), "%.0f°", temperature(summary.nightLow));
    if (std::isfinite(summary.dayHigh))
        std::snprintf(high, sizeof(high), "%.0f°", temperature(summary.dayHigh));
    const bool early = localNow % 86400 < 6 * 3600;
    std::snprintf(text, sizeof(text), "%s %s / %s %s",
                  early ? "This morning" : "Tonight", low, early ? "Today" : "Tomorrow", high);
    lv_label_set_text(w->forecast_summary_label, text);
    lv_label_set_text(w->forecast_heading_label,
                      minimum <= maximum ? "NEXT 24 HOURS" : "FORECAST UNAVAILABLE");

    lv_obj_add_flag(w->midnight_divider, LV_OBJ_FLAG_HIDDEN);
    bool dayMarked = false;
    int64_t previousDay = localNow / 86400;
    for (size_t i = 0; i < next.size(); ++i) {
        auto& ui = w->forecast_points[i];
        const auto& point = next[i];
        w->forecast_valid[i] = point.timestamp != 0;
        lv_label_set_text(ui.day, "");
        weather_icon_set(ui.icon, point.icon);
        if (!point.timestamp) {
            lv_label_set_text(ui.time, "--");
            lv_label_set_text(ui.temperature, "--°");
            continue;
        }
        time_t local = static_cast<time_t>(point.timestamp);
        struct tm info{};
        gmtime_r(&local, &info);
        int hour = info.tm_hour % 12 ? info.tm_hour % 12 : 12;
        // Forecast samples can fall on :30/:45 in fractional-offset timezones.
        if (info.tm_min)
            std::snprintf(text, sizeof(text), "%d:%02d%s", hour, info.tm_min, info.tm_hour < 12 ? "a" : "p");
        else
            std::snprintf(text, sizeof(text), "%d %s", hour, info.tm_hour < 12 ? "AM" : "PM");
        lv_label_set_text(ui.time, text);
        if (!dayMarked && point.timestamp / 86400 != previousDay) {
            char day[8];
            std::strftime(day, sizeof(day), "%a", &info);
            lv_label_set_text(ui.day, day);
            if (i) {
                lv_obj_set_x(w->midnight_divider, 24 + i * 94 - 1);
                lv_obj_clear_flag(w->midnight_divider, LV_OBJ_FLAG_HIDDEN);
            }
            dayMarked = true;
        }
        previousDay = point.timestamp / 86400;
        std::snprintf(text, sizeof(text), "%.0f°", temperature(point.temperature));
        lv_label_set_text(ui.temperature, text);
        w->forecast_curve[i].x = 71 + i * 94;
        w->forecast_curve[i].y = maximum > minimum ?
            23 - static_cast<int>((point.temperature - minimum) / (maximum - minimum) * 18.f) : 14;
    }
    lv_obj_invalidate(w->forecast_graph);
}
