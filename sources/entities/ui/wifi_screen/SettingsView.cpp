#include "SettingsView.h"
#include "entities/ui/Dashboard/KitchenTheme.h"
#include <initializer_list>

namespace {
using namespace KitchenTheme;
lv_obj_t* text(lv_obj_t* parent, const char* value, const lv_font_t* font = &lv_font_montserrat_24)
{
    auto* label = lv_label_create(parent);
    lv_label_set_text(label, value);
    lv_obj_set_style_text_font(label, font, 0);
    return label;
}

lv_obj_t* note(lv_obj_t* parent, const char* value)
{
    auto* label = text(parent, value, &lv_font_montserrat_22);
    lv_obj_set_width(label, LV_PCT(100));
    lv_obj_set_style_text_color(label, lv_color_hex(Muted), 0);
    return label;
}

lv_obj_t* row(lv_obj_t* parent, int height = 72)
{
    auto* object = lv_obj_create(parent);
    surface(object, Oatmeal, 10);
    lv_obj_set_size(object, LV_PCT(100), height);
    lv_obj_set_style_pad_hor(object, 18, 0);
    lv_obj_set_style_pad_column(object, 14, 0);
    lv_obj_set_flex_flow(object, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(object, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(object, LV_OBJ_FLAG_SCROLLABLE);
    return object;
}

lv_obj_t* button(lv_obj_t* parent, const char* value, int width = 112)
{
    auto* object = lv_btn_create(parent);
    lv_obj_set_size(object, width, 46);
    auto* label = text(object, value);
    lv_obj_center(label);
    return object;
}
}

SettingsHeaderWidgets settings_header_create(lv_obj_t* parent, lv_event_cb_t home, void* context)
{
    SettingsHeaderWidgets w{};
    w.header = lv_obj_create(parent);
    KitchenTheme::surface(w.header);
    lv_obj_set_size(w.header, LV_PCT(100), 80);
    lv_obj_clear_flag(w.header, LV_OBJ_FLAG_SCROLLABLE);
    w.home = button(w.header, LV_SYMBOL_LEFT "  Home", 128);
    lv_obj_set_pos(w.home, 28, 16);
    lv_obj_add_event_cb(w.home, home, LV_EVENT_CLICKED, context);
    auto* title = text(w.header, "Settings", &lv_font_montserrat_32);
    lv_obj_set_pos(title, 184, 14);
    auto* subtitle = text(w.header, "Display & connections", &lv_font_montserrat_18);
    lv_obj_set_pos(subtitle, 185, 49);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(KitchenTheme::Muted), 0);
    w.connection = text(w.header, "Wi-Fi offline", &lv_font_montserrat_22);
    w.location = text(w.header, "Weather location not set", &lv_font_montserrat_18);
    for (auto* label : {w.connection, w.location}) {
        lv_obj_set_width(label, 340);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_RIGHT, 0);
    }
    lv_obj_set_pos(w.connection, 432, 18);
    lv_obj_set_pos(w.location, 432, 47);
    lv_obj_set_style_text_color(w.location, lv_color_hex(KitchenTheme::Muted), 0);
    return w;
}

void settings_menu_style(lv_obj_t* menu)
{
    KitchenTheme::surface(menu);
    lv_menu_set_mode_header(menu, LV_MENU_HEADER_TOP_FIXED);
    auto* header = lv_menu_get_main_header(menu);
    KitchenTheme::surface(header);
    lv_obj_set_height(header, 58);
    lv_obj_set_style_pad_hor(header, 24, 0);
    lv_obj_set_style_text_font(header, &lv_font_montserrat_28, 0);
    lv_obj_set_style_pad_ver(header, 12, 0);
    if (auto* sidebarHeader = lv_menu_get_sidebar_header(menu)) {
        // The outer Settings header already names the screen.
        lv_obj_set_style_bg_color(sidebarHeader, lv_color_hex(KitchenTheme::Oatmeal), 0);
        lv_obj_set_height(sidebarHeader, 0);
        lv_obj_set_style_pad_all(sidebarHeader, 0, 0);
    }
    if (auto* sidebar = lv_menu_get_cur_sidebar_page(menu)) {
        auto* container = lv_obj_get_parent(sidebar);
        KitchenTheme::surface(container, KitchenTheme::Oatmeal, 12);
        lv_obj_set_width(container, 198);
        lv_obj_set_style_pad_all(container, 12, 0);
        lv_obj_set_style_pad_all(sidebar, 0, 0);
        lv_obj_set_style_pad_row(sidebar, 8, 0);
    }
}

void settings_page_style(lv_obj_t* page)
{
    lv_obj_set_style_bg_color(page, lv_color_hex(KitchenTheme::Paper), 0);
    lv_obj_set_style_pad_hor(page, 24, 0);
    lv_obj_set_style_pad_top(page, 8, 0);
    lv_obj_set_style_pad_bottom(page, 28, 0);
    lv_obj_set_style_pad_row(page, 18, 0);
    lv_obj_set_scroll_dir(page, LV_DIR_VER);
}

void settings_sidebar_item_style(lv_obj_t* item)
{
    lv_obj_set_size(item, LV_PCT(100), 56);
    lv_obj_set_style_pad_hor(item, 14, 0);
    lv_obj_set_style_pad_ver(item, 10, 0);
    lv_obj_set_style_text_font(item, &lv_font_montserrat_22, 0);
    lv_obj_set_flex_align(item, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
}

SettingsUnitsWidgets settings_units_create(lv_obj_t* page)
{
    note(page, "Choose how temperatures and pressure appear across the display.");
    auto* temperature = row(page);
    text(temperature, "Temperature");
    auto* temperatureDropdown = lv_dropdown_create(temperature);
    lv_dropdown_set_options(temperatureDropdown, "Celsius\nFahrenheit");
    lv_obj_set_size(temperatureDropdown, 212, 48);
    auto* pressure = row(page);
    text(pressure, "Pressure");
    auto* pressureDropdown = lv_dropdown_create(pressure);
    lv_dropdown_set_options(pressureDropdown, "hPa\nmmHg");
    lv_obj_set_size(pressureDropdown, 212, 48);
    return {temperatureDropdown, pressureDropdown};
}

SettingsBrightnessWidgets settings_brightness_create(lv_obj_t* page)
{
    note(page, "Let the display adjust through the day, or choose a fixed brightness.");
    auto* automatic = row(page);
    text(automatic, "Automatic brightness");
    auto* toggle = lv_switch_create(automatic);
    lv_obj_set_size(toggle, 56, 32);
    lv_obj_add_state(toggle, LV_STATE_CHECKED);
    auto* manual = row(page, 100);
    lv_obj_set_flex_flow(manual, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_ver(manual, 16, 0);
    auto* labels = lv_obj_create(manual);
    KitchenTheme::surface(labels, KitchenTheme::Oatmeal);
    lv_obj_set_size(labels, LV_PCT(100), 30);
    lv_obj_set_flex_flow(labels, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(labels, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(labels, LV_OBJ_FLAG_SCROLLABLE);
    text(labels, "Manual brightness");
    auto* value = text(labels, "17%");
    auto* slider = lv_slider_create(manual);
    lv_obj_set_width(slider, LV_PCT(94));
    lv_obj_set_height(slider, 8);
    lv_slider_set_value(slider, 17, LV_ANIM_OFF);
    lv_obj_add_state(slider, LV_STATE_DISABLED);
    note(page, "Turn off automatic brightness to use the slider.");
    return {toggle, slider, value};
}

lv_obj_t* settings_location_create(lv_obj_t* page, const char* city)
{
    note(page, "Your location sets the weather forecast and local time zone.");
    auto* label = text(page, "City");
    lv_obj_set_width(label, LV_PCT(100));
    lv_obj_set_style_text_color(label, lv_color_hex(KitchenTheme::Muted), 0);
    auto* entry = lv_textarea_create(page);
    lv_textarea_set_one_line(entry, true);
    lv_textarea_set_placeholder_text(entry, "Enter a city");
    lv_textarea_set_text(entry, city);
    lv_obj_set_width(entry, LV_PCT(100));
    lv_obj_set_height(entry, 54);
    note(page, "Tap to edit, then use the keyboard checkmark to save. Wi-Fi is needed to check the city.");
    return entry;
}

SettingsTimeWidgets settings_time_create(lv_obj_t* page, int year, int month, int day)
{
    SettingsTimeWidgets w{};
    note(page, "Network time updates automatically. You can also set the clock manually.");
    auto* steppers = row(page, 166);
    static const char* names[] = {"Hours", "Minutes", "Seconds"};
    for (int i = 0; i < 3; ++i) {
        auto* column = lv_obj_create(steppers);
        KitchenTheme::surface(column, KitchenTheme::Oatmeal);
        lv_obj_set_size(column, 112, 150);
        lv_obj_set_flex_flow(column, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(column, 4, 0);
        lv_obj_set_flex_align(column, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
        lv_obj_clear_flag(column, LV_OBJ_FLAG_SCROLLABLE);
        text(column, names[i], &lv_font_montserrat_18);
        w.up[i] = button(column, LV_SYMBOL_UP, 80);
        lv_obj_set_height(w.up[i], 36);
        lv_obj_set_style_min_height(w.up[i], 36, 0);
        w.values[i] = text(column, "00", &lv_font_montserrat_32);
        w.down[i] = button(column, LV_SYMBOL_DOWN, 80);
        lv_obj_set_height(w.down[i], 36);
        lv_obj_set_style_min_height(w.down[i], 36, 0);
    }
    w.apply = button(page, "Set time", 148);
    note(page, "Tap a calendar date below to set the date immediately.");
    w.calendar = lv_calendar_create(page);
    lv_obj_set_size(w.calendar, LV_PCT(100), 300);
    lv_calendar_header_dropdown_create(w.calendar);
    lv_calendar_header_dropdown_set_year_list(w.calendar,
        "2050\n2049\n2048\n2047\n2046\n2045\n2044\n2043\n2042\n2041\n2040"
        "\n2039\n2038\n2037\n2036\n2035\n2034\n2033\n2032\n2031\n2030"
        "\n2029\n2028\n2027\n2026\n2025");
    lv_calendar_set_today_date(w.calendar, year, month, day);
    lv_calendar_set_showed_date(w.calendar, year, month);
    return w;
}

void settings_network_style(lv_obj_t* button, lv_obj_t* row, lv_obj_t* image)
{
    KitchenTheme::surface(button, KitchenTheme::Oatmeal, 10);
    lv_obj_set_style_pad_hor(button, 16, 0);
    lv_obj_set_style_pad_ver(button, 4, 0);
    lv_obj_set_style_pad_ver(row, 14, 0);
    lv_obj_set_style_text_font(row, &lv_font_montserrat_22, 0);
    if (image) {
        lv_obj_set_style_img_recolor(image, lv_color_hex(KitchenTheme::Ink), 0);
        lv_obj_set_style_img_recolor_opa(image, LV_OPA_COVER, 0);
    }
}

void settings_keyboard_layout(lv_obj_t* content, bool visible)
{
    if (visible) {
        lv_obj_set_flex_grow(content, 0);
        // LVGL 8 clears h_layout during the parent's next layout pass. Clear
        // that flag before changing height, otherwise the old grown size sticks.
        lv_obj_update_layout(lv_obj_get_parent(content));
        lv_obj_set_height(content, lv_disp_get_ver_res(lv_obj_get_disp(content)) / 2 - 80);
    } else {
        lv_obj_set_flex_grow(content, 1);
    }
    lv_obj_set_style_pad_bottom(content, visible ? 4 : 20, 0);
    lv_obj_update_layout(lv_obj_get_parent(content));
}
