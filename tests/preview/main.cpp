#include "entities/ui/Dashboard/DashboardWidgets.h"
#include "entities/ui/Dashboard/WeatherIcon.h"
#include "entities/ui/Dashboard/KitchenTheme.h"
#include "entities/ui/wifi_screen/SettingsView.h"
#include "entities/IndoorMetricHistory.h"

#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

namespace {
constexpr int Width = 800, Height = 480;
std::array<lv_color_t, Width * Height> framebuffer;
std::array<lv_color_t, Width * 48> drawBuffer;

void flush(lv_disp_drv_t* driver, const lv_area_t* area, lv_color_t* pixels)
{
    for (int y = area->y1; y <= area->y2; ++y)
        for (int x = area->x1; x <= area->x2; ++x)
            framebuffer[y * Width + x] = *pixels++;
    lv_disp_flush_ready(driver);
}

void saveBitmap(const std::string& path)
{
    std::ofstream out(path, std::ios::binary);
    const auto u16 = [&out](uint16_t n) { out.put(n & 255); out.put(n >> 8); };
    const auto u32 = [&out](uint32_t n) { for (int i = 0; i < 4; ++i) out.put((n >> (i * 8)) & 255); };
    out.write("BM", 2);
    u32(54 + Width * Height * 3); u32(0); u32(54);
    u32(40); u32(Width); u32(Height); u16(1); u16(24);
    u32(0); u32(Width * Height * 3); u32(0); u32(0); u32(0); u32(0);
    for (int y = Height - 1; y >= 0; --y)
        for (int x = 0; x < Width; ++x)
        {
            lv_color32_t color;
            color.full = lv_color_to32(framebuffer[y * Width + x]);
            out.put(color.ch.blue); out.put(color.ch.green); out.put(color.ch.red);
        }
    assert(out.good());
}

void snapshot(const std::string& path)
{
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(nullptr);
    saveBitmap(path);
}

void checkHomeBounds(lv_obj_t* obj)
{
    if (lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN)) return;
    lv_area_t area;
    lv_obj_get_coords(obj, &area);
    if (!(area.x1 >= 0 && area.y1 >= 0 && area.x2 < Width && area.y2 < Height))
    {
        std::cerr << "Object out of bounds: " << area.x1 << ',' << area.y1 << " to "
                  << area.x2 << ',' << area.y2 << '\n';
        assert(false);
    }
    if (lv_obj_check_type(obj, &lv_label_class))
    {
        lv_point_t size;
        lv_txt_get_size(&size, lv_label_get_text(obj), lv_obj_get_style_text_font(obj, 0),
                        0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
        if (size.x > lv_obj_get_width(obj))
        {
            std::cerr << "Clipped label: " << lv_label_get_text(obj) << ", needs " << size.x
                      << ", has " << lv_obj_get_width(obj) << '\n';
            assert(false);
        }
    }
    for (uint32_t i = 0; i < lv_obj_get_child_cnt(obj); ++i)
        checkHomeBounds(lv_obj_get_child(obj, i));
}
}

int main(int argc, char** argv)
{
    lv_init();
    lv_disp_draw_buf_t buffer;
    lv_disp_draw_buf_init(&buffer, drawBuffer.data(), nullptr, drawBuffer.size());
    lv_disp_drv_t driver;
    lv_disp_drv_init(&driver);
    driver.hor_res = Width; driver.ver_res = Height;
    driver.draw_buf = &buffer; driver.flush_cb = flush;
    lv_disp_t* display = lv_disp_drv_register(&driver);
    KitchenTheme::initialize(display);
    auto* w = dashboard_view_create(lv_scr_act(), 0);
    assert(w);
    constexpr int64_t now = 1793090520; // Tue Oct 27 2026, 08:42, location local.
    const int64_t midnight = now - now % 86400;
    WeatherForecast forecast(now);
    const float temperatures[] = {6, 8, 7, 3, 1, -1, -2, 0, 3, 6, 9, 5};
    const char* icons[] = {"02d", "01d", "03d", "04d", "02n", "01n", "01n", "02n", "02d", "01d", "01d", "03d"};
    for (size_t i = 0; i < std::size(temperatures); ++i)
        forecast.add(midnight + (9 + i * 3) * 3600, temperatures[i], icons[i]);

    dashboard_view_update_clock(w, now);
    lv_label_set_text(w->temp_inside_label, "21°");
    lv_label_set_text(w->indoor_humidity_label, "42%");
    lv_label_set_text(w->temp_outside_label, "6°");
    lv_label_set_text(w->feels_like_label, "Feels like 3°C");
    lv_obj_set_style_text_opa(w->wifi_label, LV_OPA_COVER, 0);
    weather_icon_set(w->outside_icon, "02d");
    dashboard_view_update_forecast(w, forecast, now, false);
    lv_obj_update_layout(w->cont);
    assert(lv_obj_get_height(w->tab_dashboard) == Height);
    assert(lv_obj_get_width(w->tab_dashboard) == Width);
    assert(std::strcmp(lv_label_get_text(w->label), "8:42") == 0);
    assert(std::strcmp(lv_label_get_text(w->forecast_points[7].temperature), "0°") == 0);
    assert(!lv_obj_has_flag(w->midnight_divider, LV_OBJ_FLAG_HIDDEN));
    checkHomeBounds(w->tab_dashboard);
    const std::string prefix = argc > 1 ? argv[1] : "dashboard";
    snapshot(prefix + ".bmp");

    // Test the widest clock values throughout a full day, including noon/midnight.
    for (int minute = 0; minute < 1440; ++minute)
    {
        dashboard_view_update_clock(w, midnight + minute * 60);
        lv_obj_update_layout(w->cont);
        if (lv_obj_get_x(w->am_pm_label) + lv_obj_get_width(w->am_pm_label) > 513)
        {
            std::cerr << "Clock overlaps weather: " << lv_label_get_text(w->label)
                      << ", clock width " << lv_obj_get_width(w->label)
                      << ", AM/PM x " << lv_obj_get_x(w->am_pm_label)
                      << ", AM/PM width " << lv_obj_get_width(w->am_pm_label) << '\n';
            assert(false);
        }
    }
    assert(std::strcmp(lv_label_get_text(w->label), "11:59") == 0);
    dashboard_view_update_clock(w, midnight);
    assert(std::strcmp(lv_label_get_text(w->label), "12:00") == 0);
    assert(std::strcmp(lv_label_get_text(w->am_pm_label), "AM") == 0);
    dashboard_view_update_clock(w, midnight + 12 * 3600);
    assert(std::strcmp(lv_label_get_text(w->am_pm_label), "PM") == 0);

    dashboard_view_update_clock(w, midnight + 23 * 3600 + 58 * 60);
    dashboard_view_update_forecast(w, forecast, midnight + 23 * 3600 + 58 * 60, true);
    assert(std::strcmp(lv_label_get_text(w->forecast_points[0].temperature), "30°") == 0);
    lv_label_set_text(w->temp_inside_label, "70°");
    lv_label_set_text(w->temp_outside_label, "-40°");
    lv_obj_set_style_text_font(w->temp_outside_label, &lv_font_montserrat_48, 0);
    lv_label_set_text(w->feels_like_label, "Feels like -45°F");
    weather_icon_set(w->outside_icon, "13n");
    lv_obj_update_layout(w->cont);
    checkHomeBounds(w->tab_dashboard);
    snapshot(prefix + "-night-fahrenheit.bmp");

    WeatherForecast missing(now);
    dashboard_view_update_forecast(w, missing, now, false);
    assert(std::strcmp(lv_label_get_text(w->forecast_heading_label), "FORECAST UNAVAILABLE") == 0);
    for (bool valid : w->forecast_valid) assert(!valid);
    assert(lv_obj_has_flag(w->midnight_divider, LV_OBJ_FLAG_HIDDEN));
    lv_obj_update_layout(w->cont);
    checkHomeBounds(w->tab_dashboard);
    snapshot(prefix + "-unavailable.bmp");

    // Actual click handlers must reach details/settings and return home.
    lv_event_send(w->indoor_button, LV_EVENT_CLICKED, nullptr);
    assert(lv_tabview_get_tab_act(w->cont) == 1);
    assert(lv_obj_get_scroll_x(lv_tabview_get_content(w->cont)) == Width);
    lv_event_send(w->status_back_button, LV_EVENT_CLICKED, nullptr);
    assert(lv_tabview_get_tab_act(w->cont) == 0);
    assert(lv_obj_get_scroll_x(lv_tabview_get_content(w->cont)) == 0);
    lv_event_send(w->settings_button, LV_EVENT_CLICKED, nullptr);
    assert(lv_tabview_get_tab_act(w->cont) == 2);
    assert(lv_obj_get_scroll_x(lv_tabview_get_content(w->cont)) == 2 * Width);

    // Render the actual sensor cards with representative live values/history.
    lv_tabview_set_act(w->cont, 1, LV_ANIM_OFF);
    const char* metrics[] = {"21.4°C", "42%", "1013 hPa", "618 ppm", "74 ppb", "28"};
    for (int i = 0; i < 6; ++i) {
        lv_label_set_text(w->status_metrics[i].value, metrics[i]);
        lv_chart_set_range(w->status_metrics[i].chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
        assert(lv_chart_get_point_count(w->status_metrics[i].chart) == IndoorMetricHistory::PointCount);
        for (size_t p = 0; p < IndoorMetricHistory::PointCount; ++p)
            lv_chart_set_value_by_id(w->status_metrics[i].chart, w->status_metrics[i].series,
                                     p, 35 + (p * 3 + i * 5) % 30);
    }
    lv_obj_update_layout(w->cont);
    checkHomeBounds(w->tab_status);
    snapshot(prefix + "-sensors.bmp");

    // Settings use the same view factories and theme as the device. Hardware
    // callbacks are excluded: these checks never save settings or use the radio.
    lv_tabview_set_act(w->cont, 2, LV_ANIM_OFF);
    auto* shell = lv_obj_create(w->tab_settings);
    KitchenTheme::surface(shell);
    lv_obj_set_size(shell, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(shell, LV_FLEX_FLOW_COLUMN);
    lv_obj_clear_flag(shell, LV_OBJ_FLAG_SCROLLABLE);
    auto header = settings_header_create(shell, [](lv_event_t* event) {
        auto* view = static_cast<DashboardWidgets*>(lv_event_get_user_data(event));
        lv_tabview_set_act(view->cont, 0, LV_ANIM_OFF);
    }, w);
    lv_label_set_text(header.connection, LV_SYMBOL_WIFI "  Kitchen Wi-Fi");
    lv_label_set_text(header.location, "Regina, CA / 8:42 AM");
    auto* content = lv_obj_create(shell);
    KitchenTheme::surface(content);
    lv_obj_set_width(content, LV_PCT(100));
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_style_pad_hor(content, 28, 0);
    lv_obj_set_style_pad_bottom(content, 20, 0);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);
    auto* menu = lv_menu_create(content);
    lv_obj_set_size(menu, LV_PCT(100), LV_PCT(100));
    settings_menu_style(menu); // Must also work before a sidebar exists.
    auto* root = lv_menu_page_create(menu, nullptr);
    lv_obj_t* pages[5];
    lv_obj_t* items[5];
    const char* titles[] = {"Units", "Date & time", "Brightness", "Location", "Wi-Fi"};
    for (int i = 0; i < 5; ++i) {
        pages[i] = lv_menu_page_create(menu, const_cast<char*>(titles[i]));
        settings_page_style(pages[i]);
        items[i] = lv_menu_cont_create(root);
        settings_sidebar_item_style(items[i]);
        auto* title = lv_label_create(items[i]);
        lv_label_set_text(title, titles[i]);
        lv_obj_set_flex_grow(title, 1);
        lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
        lv_menu_set_load_page_event(menu, items[i], pages[i]);
    }
    auto units = settings_units_create(pages[0]);
    auto time = settings_time_create(pages[1], 2026, 10, 27);
    lv_label_set_text(time.values[0], "08");
    lv_label_set_text(time.values[1], "42");
    lv_label_set_text(time.values[2], "00");
    auto brightness = settings_brightness_create(pages[2]);
    auto* city = settings_location_create(pages[3], "Regina");
    for (const char* ssid : {"Kitchen Wi-Fi", "Guest network", "A very long nearby network name"}) {
        auto* network = lv_btn_create(pages[4]);
        lv_obj_set_size(network, LV_PCT(100), 64);
        auto* name = lv_label_create(network);
        lv_label_set_text_fmt(name, LV_SYMBOL_WIFI "  %s", ssid);
        lv_obj_set_width(name, LV_PCT(100));
        lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
        settings_network_style(network, network, nullptr);
        lv_obj_center(name);
    }
    lv_menu_set_sidebar_page(menu, root);
    settings_menu_style(menu);
    for (int i = 0; i < 5; ++i) {
        lv_event_send(items[i], LV_EVENT_CLICKED, nullptr);
        assert(lv_menu_get_cur_main_page(menu) == pages[i]);
        lv_obj_update_layout(w->cont);
        assert(lv_obj_get_width(pages[i]) >= 480);
        snapshot(prefix + "-settings-" + std::to_string(i) + ".bmp");
    }
    lv_event_send(items[0], LV_EVENT_CLICKED, nullptr);
    lv_dropdown_open(units.temperature);
    auto* list = lv_dropdown_get_list(units.temperature);
    assert(list && lv_obj_is_visible(list));
    assert(lv_obj_get_style_bg_color(list, 0).full == lv_color_hex(KitchenTheme::Oatmeal).full);
    snapshot(prefix + "-settings-dropdown.bmp");
    lv_dropdown_close(units.temperature);

    lv_event_send(items[1], LV_EVENT_CLICKED, nullptr);
    lv_obj_scroll_to_view_recursive(time.calendar, LV_ANIM_OFF);
    lv_obj_update_layout(w->cont);
    snapshot(prefix + "-settings-calendar.bmp");

    lv_event_send(items[3], LV_EVENT_CLICKED, nullptr);
    settings_keyboard_layout(content, true);
    auto* keyboard = lv_keyboard_create(w->tab_settings);
    lv_obj_set_size(keyboard, Width, Height / 2);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_keyboard_set_textarea(keyboard, city);
    lv_obj_update_layout(w->cont);
    lv_obj_scroll_to_view_recursive(city, LV_ANIM_OFF);
    lv_obj_update_layout(w->cont);
    lv_area_t field;
    lv_obj_get_coords(city, &field);
    snapshot(prefix + "-settings-keyboard.bmp");
    if (field.y2 >= Height / 2)
        std::cerr << "Keyboard field bounds: " << field.y1 << ".." << field.y2
                  << ", content height " << lv_obj_get_height(content)
                  << ", page height " << lv_obj_get_height(pages[3])
                  << ", page scroll " << lv_obj_get_scroll_y(pages[3]) << '\n';
    assert(field.y2 < Height / 2);
    lv_obj_del(keyboard);
    settings_keyboard_layout(content, false);
    lv_event_send(header.home, LV_EVENT_CLICKED, nullptr);
    assert(lv_tabview_get_tab_act(w->cont) == 0);
    std::cout << "Dashboard layout, clock, forecast, units, missing data, and navigation passed.\n";
}
