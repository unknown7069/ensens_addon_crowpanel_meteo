#include "WifiScreenHeader.h"
#include "entities/ui/Dashboard/Dashboard.h"

#include <cstdio>
#include <ctime>

void WifiScreenHeader::homeCallback(lv_event_t*)
{
    lvgl_port_lock(-1);
    WifiScreen::instance().hideKeyboard();
    if (auto* view = Dashboard::instance().getTabView())
        lv_tabview_set_act(view->cont, 0, LV_ANIM_OFF);
    lvgl_port_unlock();
}

void WifiScreenHeader::create(lv_obj_t* parent)
{
    widgets = settings_header_create(parent, homeCallback, this);
}

void WifiScreenHeader::setSSID(char* ssid)
{
    lvgl_port_lock(-1);
    if (ssid && *ssid)
        lv_label_set_text_fmt(widgets.connection, LV_SYMBOL_WIFI "  %s", ssid);
    else
        lv_label_set_text(widgets.connection, "Wi-Fi offline");
    lvgl_port_unlock();
}

void WifiScreenHeader::updateRSSI(int8_t rssi)
{
    lvgl_port_lock(-1);
    lv_obj_set_style_text_opa(widgets.connection, rssi >= -70 ? LV_OPA_COVER : LV_OPA_70, 0);
    lvgl_port_unlock();
}

void WifiScreenHeader::updateLocation()
{
    char context[112];
    if (!*city_)
        std::snprintf(context, sizeof(context), "Weather location not set");
    else
        std::snprintf(context, sizeof(context), "%s%s%s%s%s", city_, *country_ ? ", " : "",
                       country_, *time_ ? " / " : "", time_);
    lv_label_set_text(widgets.location, context);
}

void WifiScreenHeader::setCity(char* city)
{
    lvgl_port_lock(-1);
    std::snprintf(city_, sizeof(city_), "%s", city ? city : "");
    updateLocation();
    lvgl_port_unlock();
}

void WifiScreenHeader::setCountry(char* country)
{
    lvgl_port_lock(-1);
    std::snprintf(country_, sizeof(country_), "%s", country ? country : "");
    updateLocation();
    lvgl_port_unlock();
}

void WifiScreenHeader::setCurrentTime(uint32_t timestamp)
{
    lvgl_port_lock(-1);
    const time_t local = timestamp;
    struct tm info{};
    if (timestamp) {
        gmtime_r(&local, &info);
        std::snprintf(time_, sizeof(time_), "%d:%02d %s",
                       info.tm_hour % 12 ? info.tm_hour % 12 : 12, info.tm_min,
                       info.tm_hour < 12 ? "AM" : "PM");
    } else {
        time_[0] = 0;
    }
    updateLocation();
    lvgl_port_unlock();
}
