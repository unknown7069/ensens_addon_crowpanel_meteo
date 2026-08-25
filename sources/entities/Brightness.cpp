#include "Brightness.h"
#include "entities/CurrentTime.h"
#include "entities/ui/wifi_screen/WifiScreen.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "lvgl.h" // For lv_timer_t and related functions

Brightness::Brightness() : brightnessTimer(NULL) {}

static void brightnessTimerCallback(lv_timer_t* timer)
{
    LV_UNUSED(timer);
    Brightness::instance().update();
}

void Brightness::createMask()
{
    lvgl_port_lock();
    mask = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(mask);
    lv_obj_set_size(mask, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(mask, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(mask, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(mask, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_move_foreground(mask);
    lvgl_port_unlock();
}

void Brightness::setMaskOpacity(uint8_t percent)
{
    if (!mask)
        return;
    if (percent > 100)
        percent = 100;
    level_ = percent;

    uint8_t opa = 255 - (percent * 255 / 100);
    lvgl_port_lock();
    lv_obj_set_style_bg_opa(mask, opa, 0);
    lvgl_port_unlock();
}

bool Brightness::init()
{
    nvs_handle_t nvsHandle;
    bool         retVal = true;
    level_              = DefaultValueWhenNotConfigured;
    autoUpdate_         = true;

    if (nvs_open("storage", NVS_READWRITE, &nvsHandle) != ESP_OK)
    {
        ESP_LOGE(TAG, "error nvs_open()");
        retVal = false;
    } else
    {
        uint8_t storedLevel = 100;
        if (nvs_get_u8(nvsHandle, "brightness", &storedLevel) == ESP_OK)
        {
            autoUpdate_ = false;
            level_      = storedLevel;
            ESP_LOGI(TAG, "Loaded user brightness level: %d", level_);
        } else
            ESP_LOGI(TAG, "Brightness not set, use auto, default: %d", level_);

        nvs_close(nvsHandle);
    }

     createMask();

     brightnessTimer = lv_timer_create(brightnessTimerCallback, AutoUpdatePeriodMs, NULL);
     if (brightnessTimer == NULL) {
         ESP_LOGE(TAG, "Failed to create brightness timer");
     }

     if (autoUpdate_) {
         update();
     } else {
         setMaskOpacity(level_);
         WifiScreen::instance().setBrightness(autoUpdate_, level_);
     }

    return retVal;
}

bool Brightness::set(bool isAuto, uint8_t percent)
{
    bool retVal = true;

    level_      = percent;
    autoUpdate_ = isAuto;

    if (isAuto)
    {
        nvs_handle_t nvsHandle;
        if ((nvs_open("storage", NVS_READWRITE, &nvsHandle) != ESP_OK) ||
            (nvs_erase_key(nvsHandle, "brightness") != ESP_OK) || (nvs_commit(nvsHandle) != ESP_OK))
        {
            ESP_LOGE(TAG, "Failed to erase brightness key");
            retVal = false;
        } else
            ESP_LOGD(TAG, "Switched to auto brightness mode");
         nvs_close(nvsHandle);
         update();
    } else
    {
        nvs_handle_t nvsHandle;
        if ((nvs_open("storage", NVS_READWRITE, &nvsHandle) != ESP_OK) ||
            (nvs_set_u8(nvsHandle, "brightness", percent) != ESP_OK) ||
            (nvs_commit(nvsHandle) != ESP_OK))
        {
            ESP_LOGE(TAG, "Failed to save brightness");
            retVal = false;
        } else
        {
            ESP_LOGI(TAG, "New manually configured brightness stored");
        }
        nvs_close(nvsHandle);

        setMaskOpacity(level_);
        ESP_LOGI(TAG, "Brightness manually configured %d", level_);

        WifiScreen::instance().setBrightness(autoUpdate_, level_);
    }
    return retVal;
}

bool Brightness::update()
{
    if (!autoUpdate_)
        return true;

    bool timeConfigured = CurrentTime::instance().isTimeSet();
    uint8_t calculatedBrightness = DefaultValueWhenNotConfigured;

    if (timeConfigured)
    {
        time_t    now = CurrentTime::instance().nowLocal();
        struct tm timeinfo;
        localtime_r(&now, &timeinfo);

        int hour = timeinfo.tm_hour;
        if (hour >= DayStartHour && hour < EveningStartHour)
        {
            calculatedBrightness = DayBrightnessPercent;
        } else if (hour >= EveningStartHour && hour < NightStartHour)
        {
            calculatedBrightness = EveningBrightnessPercent;
        } else
        {
            calculatedBrightness = NightBrightnessPercent;
        }

        ESP_LOGD(TAG, "Auto brightness based on time(%02d): %d", hour, calculatedBrightness);
    } else
    {
        calculatedBrightness = DefaultValueWhenNotConfigured;
        ESP_LOGI(TAG, "Time not configured, using default: %d", calculatedBrightness);
    }

    level_ = calculatedBrightness;
    setMaskOpacity(level_);
    WifiScreen::instance().setBrightness(autoUpdate_, level_);

    return true;
}

bool Brightness::get(bool* isAuto, uint8_t* percent)
{
    if (!isAuto || !percent)
    {
        ESP_LOGE(TAG, "percent pointer is null");
        return false;
    }

    *isAuto  = autoUpdate_;
    *percent = level_;

    return true;
}