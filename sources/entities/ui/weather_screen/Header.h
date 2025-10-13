#include "adapters/lvgl/FlexContainer.h"
#include "adapters/lvgl/Image.h"
#include "entities/ui/weather_screen/elements/LocationLabel.h"
#include "entities/ui/weather_screen/elements/TimeZoneLabel.h"
#include "entities/ui/weather_screen/elements/WifiLabel.h"
#include <esp_log.h>

LV_IMG_DECLARE(wifi_100);
LV_IMG_DECLARE(wifi_75);
LV_IMG_DECLARE(wifi_50);
LV_IMG_DECLARE(wifi_25);

class Header
{
    WifiLabel              wifiLabel;
    LocationLabel          location;
    TimeZoneLabel          timeLabel;
    TimeZoneLabel          dateLabel;
    FlexContainer          header;
    FlexContainer          wifiInfoContainer;
    FlexContainer          locationContainer;
    Image                  wifiIcon;
    static constexpr char* Tag = "Header";

public:
    enum WifiContainerConfig
    {
        ShowWifiIconAndName = 1,
        ShowWifiIconOnly
    };
    Header(WifiContainerConfig config) : wifiContainerConfig(config)
    {
        ;
    }
    void create(FlexContainer& parent)
    {
        // create header.
        header.create(parent.get(), lv_pct(100), lv_pct(8), LV_FLEX_FLOW_ROW);
        header.align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_clear_flag(header.get(), LV_OBJ_FLAG_SCROLLABLE);
        {
            /*******************/
            /** WIFI CONTAINER */
            /*******************/
            wifiInfoContainer.create(header.get(), lv_pct(10), lv_pct(100), LV_FLEX_FLOW_ROW);
            wifiInfoContainer.align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                                    LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(wifiInfoContainer.get(), LV_OBJ_FLAG_SCROLLABLE);
            wifiInfoContainer.padding(5, 5);
            wifiInfoContainer.paddingGap(5);
            {
                // create wifi image.
                wifiIcon.create(wifiInfoContainer.get());
                wifiIcon.set(&wifi_25);
                wifiIcon.align(LV_ALIGN_TOP_LEFT);

                // create wifi label.
                if (wifiContainerConfig != ShowWifiIconOnly)
                    wifiLabel.create(wifiInfoContainer.get(), &lv_font_montserrat_14);
            }

            /************************************/
            /** LOCATION CENTRAL LOCATION LABEL */
            /************************************/
            locationContainer.create(header.get(), lv_pct(90), lv_pct(100), LV_FLEX_FLOW_ROW);
            lv_obj_clear_flag(locationContainer.get(), LV_OBJ_FLAG_SCROLLABLE);
            locationContainer.align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                                    LV_FLEX_ALIGN_CENTER);
            locationContainer.paddingGap(30);
            locationContainer.padding(10, 0);
            {
                // create location label.
                location.create(locationContainer.get(), &lv_font_montserrat_16);
                timeLabel.create(locationContainer.get(), &lv_font_montserrat_16);
                dateLabel.create(locationContainer.get(), &lv_font_montserrat_16);
            }
        }
    }

    void updateRSSI(int8_t rssi)
    {
        if (rssi >= -50)
            wifiIcon.set(&wifi_100);
        else if (rssi >= -60)
            wifiIcon.set(&wifi_75);
        else if (rssi >= -70)
            wifiIcon.set(&wifi_50);
        else
            wifiIcon.set(&wifi_25);
    }

    void setSSID(char* newSSID)
    {
        if (wifiContainerConfig != ShowWifiIconOnly)
            wifiLabel.setSSID(newSSID);
        if (!newSSID)
            wifiIcon.set(&wifi_25);
    }

    void setCity(char* city)
    {
        location.setCity(city);
    }

    void setCountry(char* country)
    {
        location.setCountry(country);
        ESP_LOGD(Tag, "container height - %d", lv_obj_get_content_height(header.get()));
    }

    void setCurrentTime(uint32_t timestamp)
    {
        timeLabel.setCurrentTime(timestamp);
        dateLabel.setCurrentDate(timestamp);
    }

private:
    WifiContainerConfig wifiContainerConfig;
};
