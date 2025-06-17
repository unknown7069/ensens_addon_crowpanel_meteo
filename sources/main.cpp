/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "esp_display_panel.hpp"
#include "esp_log.h"

#include "adapters/lvgl/lvgl_port_v8.h"

#include "entities/BM8563.h"
#include "entities/EnvironmentalSensorData.h"
#include "entities/UART.h"
#include "entities/ui/Dashboard/Dashboard.h"

#include "entities/Brightness.h"
#include "entities/CurrentTime.h"
#include "entities/Location.h"
#include "entities/Units.h"
#include "entities/WIFI.h"
#include "entities/Weather.h"
#include "usecases/AccessPointsUpdate.h"
#include "entities/ui/weather_screen/WeatherScreen.h"

#include <ctime>
#include <sys/time.h>

#include "timestamp.h"

using namespace esp_panel::drivers;
using namespace esp_panel::board;

static const char* TAG = "main";

#define I2C_NUM I2C_NUM_0
#define I2C_MASTER_SDA_IO GPIO_NUM_15 /*!< gpio number for I2C master clock */
#define I2C_MASTER_SCL_IO GPIO_NUM_16 /*!< gpio number for I2C master data  */
#define I2C_MASTER_FREQ_HZ 100000     /*!< I2C master clock frequency */

extern "C" void app_main(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master =
            {
                .clk_speed = I2C_MASTER_FREQ_HZ,
            },
    };

    i2c_bus_handle_t i2c_bus = i2c_bus_create(I2C_NUM, &conf);

    BM8563::init(i2c_bus);
    if (BM8563::instance().isVoltageLow())
    {
        // need to reset time
    }
    BM8563::Time_t rtc_time = BM8563::instance().getTime();
    ESP_LOGI(TAG, "Time from BM8563 rtc: %02u:%02u:%02u", rtc_time.hours, rtc_time.minutes,
             rtc_time.seconds);

    BM8563::Date_t rtc_date = BM8563::instance().getDate();
    ESP_LOGI(TAG, "Date from BM8563 rtc: day=%u, weekDay=%u, month=%u, year=%u", rtc_date.day,
             rtc_date.weekDay, rtc_date.month, rtc_date.year);

    Board*      board     = new Board();
    static bool connected = false;

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGE(TAG, "mktime failed");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    assert(board);

    ESP_UTILS_CHECK_FALSE_EXIT(board->init(), "Board init failed");
    ESP_UTILS_CHECK_FALSE_EXIT(board->begin(), "Board begin failed");
    ESP_UTILS_CHECK_FALSE_EXIT(lvgl_port_init(board->getLCD(), board->getTouch()),
                               "LVGL init failed");
    lv_disp_t*  dispp = lv_disp_get_default();
    lv_theme_t* theme =
        lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE),
                              lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);

    Aggregator::instance().create();
    UART::instance().init();
    WIFI::instance().init();
    Brightness::instance().init();

    Location::Data* location = static_cast<Location::Data*>(
        heap_caps_calloc(1, sizeof(Location::Data), MALLOC_CAP_SPIRAM));

    TimeStamp::instance().init();
    UseCases::AccessPointsUpdate::instance().init();

    while (true)
    {
        if (!WIFI::instance().isConnected())
        {
            connected = false;
            WIFI::instance().waitForConnection();
            continue;
        }
        if ((!connected) && (!CurrentTime::instance().isTimeSet()))
        {
            CurrentTime::instance().init();
            if (CurrentTime::instance().sync())
            {
                time_t    now = CurrentTime::instance().now();
                struct tm timeinfo;
                localtime_r(&now, &timeinfo);
                char strftime_buf[64];
                strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
                ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);

                TimeStamp::instance().is_sync_current_time = 1;
            }
        }
        connected = true;

        Location::instance().get(*location);
        Weather::instance().setLocation(location->latitude, location->longitude,
                                        location->locationName);
        char   ssid[MAX_SSID_LEN + 1];
        int8_t rssi;
        WIFI::instance().getCurrentAP(ssid, &rssi);
        WeatherScreen::instance().updateRSSI(rssi);
        WeatherScreen::instance().setSSID(ssid);
        WifiScreen::instance().setSSID(ssid, rssi);

        if (WeatherScreen::instance().updateWeather())
        {
            vTaskDelay(10000);
        }
    }
}
