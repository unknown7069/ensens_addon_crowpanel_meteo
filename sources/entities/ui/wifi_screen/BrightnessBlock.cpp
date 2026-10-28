#include "BrightnessBlock.h"
#include "Brightness.h"
#include <esp_log.h>

void BrightnessBlock::create(Menu& menu)
{
    page.create(menu, "Brightness");
    page.createSidebarItem("Brightness");
    const auto view = settings_brightness_create(page.getPage());
    brightnessSwitch = view.automatic;
    brightnessSlider = view.slider;
    brightnessValue = view.value;
    lv_obj_add_event_cb(brightnessSwitch, switchCallback, LV_EVENT_VALUE_CHANGED, this);
    lv_obj_add_event_cb(brightnessSlider, sliderCallback, LV_EVENT_VALUE_CHANGED, this);
    setSliderDisabled(true);
}

void BrightnessBlock::setSliderDisabled(bool disabled)
{
    if (disabled)
    {
        lv_obj_add_state(brightnessSlider, LV_STATE_DISABLED);
        lv_obj_clear_flag(brightnessSlider, LV_OBJ_FLAG_CLICKABLE);
    } else
    {
        lv_obj_clear_state(brightnessSlider, LV_STATE_DISABLED);
        lv_obj_add_flag(brightnessSlider, LV_OBJ_FLAG_CLICKABLE);
    }
}

void BrightnessBlock::set(bool autoUpdate, uint8_t level)
{
    lvgl_port_lock();
    if (autoUpdate)
    {
        lv_obj_add_state(brightnessSwitch, LV_STATE_CHECKED);
    } else
    {
        lv_obj_clear_state(brightnessSwitch, LV_STATE_CHECKED);
    }

    setSliderDisabled(autoUpdate);

    lv_slider_set_value(brightnessSlider, level, LV_ANIM_OFF);
    static char textLevel[10] = { 0 };
    snprintf(textLevel, 10, "%d%%", level);
    lv_label_set_text(brightnessValue, textLevel);
    lvgl_port_unlock();
}

void BrightnessBlock::switchCallback(lv_event_t* e)
{
    lvgl_port_lock();
    bool             value = lv_obj_has_state(lv_event_get_current_target(e), LV_STATE_CHECKED);
    BrightnessBlock* self  = static_cast<BrightnessBlock*>(lv_event_get_user_data(e));
    ESP_LOGD(TAG, "Auto brightness: %s", value ? "ON" : "OFF");
    self->setSliderDisabled(value);
    Brightness::instance().set(value, lv_slider_get_value(self->brightnessSlider));
    lvgl_port_unlock();
}

void BrightnessBlock::sliderCallback(lv_event_t* e)
{
    lvgl_port_lock();
    int              value = lv_slider_get_value(lv_event_get_current_target(e));
    BrightnessBlock* self  = static_cast<BrightnessBlock*>(lv_event_get_user_data(e));
    ESP_LOGI(TAG, "Brightness: %d", value);
    char text[10] = { 0 };
    snprintf(text, 10, "%d%%", value);
    lv_label_set_text(self->brightnessValue, text);
    Brightness::instance().set(false, lv_slider_get_value(self->brightnessSlider));
    lvgl_port_unlock();
}
