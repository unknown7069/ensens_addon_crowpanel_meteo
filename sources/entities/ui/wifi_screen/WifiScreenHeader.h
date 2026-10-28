#pragma once

#include "SettingsView.h"
#include <cstdint>

class WifiScreenHeader {
    SettingsHeaderWidgets widgets{};
    char city_[64]{};
    char country_[8]{};
    char time_[16]{};
    void updateLocation();
    static void homeCallback(lv_event_t* event);

public:
    void create(lv_obj_t* parent);
    void updateRSSI(int8_t rssi);
    void setSSID(char* ssid);
    void setCity(char* city);
    void setCountry(char* country);
    void setCurrentTime(uint32_t timestamp);
};
