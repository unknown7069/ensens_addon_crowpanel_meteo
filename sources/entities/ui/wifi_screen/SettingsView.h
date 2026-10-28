#pragma once

#include "lvgl.h"

// Pure LVGL views shared by firmware and the desktop render checks.
struct SettingsHeaderWidgets {
    lv_obj_t* header;
    lv_obj_t* home;
    lv_obj_t* connection;
    lv_obj_t* location;
};
struct SettingsUnitsWidgets { lv_obj_t* temperature; lv_obj_t* pressure; };
struct SettingsBrightnessWidgets { lv_obj_t* automatic; lv_obj_t* slider; lv_obj_t* value; };
struct SettingsTimeWidgets {
    lv_obj_t* calendar;
    lv_obj_t* values[3];
    lv_obj_t* up[3];
    lv_obj_t* down[3];
    lv_obj_t* apply;
};

SettingsHeaderWidgets settings_header_create(lv_obj_t* parent, lv_event_cb_t home, void* context);
void settings_menu_style(lv_obj_t* menu);
void settings_page_style(lv_obj_t* page);
void settings_sidebar_item_style(lv_obj_t* item);
SettingsUnitsWidgets settings_units_create(lv_obj_t* page);
SettingsBrightnessWidgets settings_brightness_create(lv_obj_t* page);
lv_obj_t* settings_location_create(lv_obj_t* page, const char* city);
SettingsTimeWidgets settings_time_create(lv_obj_t* page, int year, int month, int day);
void settings_network_style(lv_obj_t* button, lv_obj_t* row, lv_obj_t* image);
void settings_keyboard_layout(lv_obj_t* content, bool visible);
