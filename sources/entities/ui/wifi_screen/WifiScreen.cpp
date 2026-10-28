#include "WifiScreen.h"
#include "entities/Brightness.h"
#include "entities/Location.h"
#include "entities/ui/Dashboard/Dashboard.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_heap_caps.h"
#include "entities/ui/Dashboard/KitchenTheme.h"

void WifiScreen::create(SensorSettings* sensorSettings, lv_obj_t* screen_)
{
    createScreen(screen_);
    ui = new (heap_caps_malloc(sizeof(UI), MALLOC_CAP_SPIRAM)) UI;
    wifiConnectionBlock =
        new (heap_caps_malloc(sizeof(WifiConnection), MALLOC_CAP_SPIRAM)) WifiConnection;
    if (!ui || !wifiConnectionBlock)
    {
        ESP_LOGE(TAG, "malloc failed");
        return;
    }
    lock();
    ui->mainContainer.create(screen, LV_PCT(100), LV_PCT(100), LV_FLEX_FLOW_COLUMN);
    KitchenTheme::surface(ui->mainContainer.get());
    lv_obj_clear_flag(ui->mainContainer.get(), LV_OBJ_FLAG_SCROLLABLE);
    ui->mainContainer.align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    {
        /// Screen header.
        ui->header.create(ui->mainContainer.get());
        // if wifi get current == true

        ui->contentContainer.create(ui->mainContainer.get(), LV_PCT(100), 372,
                                    LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_grow(ui->contentContainer.get(), 1);
        lv_obj_set_style_pad_hor(ui->contentContainer.get(), 28, 0);
        lv_obj_set_style_pad_bottom(ui->contentContainer.get(), 20, 0);
        lv_obj_clear_flag(ui->contentContainer.get(), LV_OBJ_FLAG_SCROLLABLE);
        // contentContainer.padding(0, 0);
        ui->contentContainer.align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);

        /// Menu.
        ui->menu.create(ui->contentContainer.get());
        ui->menu.setRootBackButtonMode(LV_MENU_ROOT_BACK_BTN_DISABLED);
        ui->menu.setTransparentBackground();
        ui->menu.setSize(lv_pct(100), lv_pct(100));

        // Root page ((sidebar) left part on screen).
        ui->menu.createRootPage(nullptr);
        // menu.createRootPage(nullptr);

        /*********************
         * UNITS CONFIGURATION
         */
        ui->unitsBlock.create(ui->menu, sensorSettings);

        /*********************
         * SENSOR CONFIGURATION
         */
        ui->sensorSelectionBlock.create(ui->menu, sensorSettings);

        /*********************
         * DATE AND TIME CONFIGURATION
         */
        ui->timestampBlock.create(ui->menu);

        /*********************
         * BRIGHTNESS CONFIGURATION
         */
        ui->brightnessBlock.create(ui->menu);

        /********************
         * CITY CONFIGURATION
         */
        ui->locationConfigurationBlock.create(ui->menu, &keyboard);

        /******************
         * WIFI CONFIGURATION
         */
        wifiConnectionBlock->create(ui->menu, &keyboard);

        ui->menu.setSidebarPage(ui->menu.getRootPage());
        lv_obj_t* sidebar   = lv_menu_get_cur_sidebar_page(ui->menu.get());
        lv_obj_t* firstItem = lv_obj_get_child(sidebar, 0);
        lv_event_send(firstItem, LV_EVENT_CLICKED, nullptr);
    }

    unlock();
}

void WifiScreen::keyboardEventCallback(lv_event_t* e)
{
    lv_event_code_t code   = lv_event_get_code(e);
    WifiScreen*     screen = &WifiScreen::instance();
    if (code == LV_EVENT_READY)
    {
        screen->lock();
        if (lv_keyboard_get_textarea(screen->keyboard) ==
            screen->wifiConnectionBlock->getPasswordTextarea())
            screen->wifiConnectionBlock->passwordKeyboardEventHandler();
        else if (lv_keyboard_get_textarea(screen->keyboard) ==
                 screen->ui->locationConfigurationBlock.getCityTextarea())
            screen->ui->locationConfigurationBlock.keyboardHandler();
        screen->unlock();
    }
    if ((code == LV_EVENT_READY || code == LV_EVENT_CANCEL) && (screen->keyboard))
    {
        screen->lock();
        screen->hideKeyboard();
        screen->unlock();
    }
}

void WifiScreen::showKeyboard(lv_obj_t* textarea)
{
    if (!keyboard) {
        keyboard = lv_keyboard_create(screen);
        lv_obj_set_size(keyboard, LV_HOR_RES, LV_VER_RES / 2);
        lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_obj_add_event_cb(keyboard, keyboardEventCallback, LV_EVENT_ALL, nullptr);
    }
    settings_keyboard_layout(ui->contentContainer.get(), true);
    lv_keyboard_set_textarea(keyboard, textarea);
    lv_obj_update_layout(ui->mainContainer.get());
    lv_obj_scroll_to_view_recursive(textarea, LV_ANIM_OFF);
}

void WifiScreen::hideKeyboard()
{
    if (keyboard) {
        auto* oldKeyboard = keyboard;
        keyboard = nullptr;
        lv_obj_remove_event_cb(oldKeyboard, keyboardEventCallback);
        lv_obj_del(oldKeyboard);
    }
    settings_keyboard_layout(ui->contentContainer.get(), false);
}

void WifiScreen::textareaEventCallback(lv_event_t* e)
{
    const auto code = lv_event_get_code(e);
    auto* self = &WifiScreen::instance();
    self->lock();
    bool open = true;
    if (lv_event_get_current_target(e) == self->ui->locationConfigurationBlock.getCityTextarea())
        open = self->ui->locationConfigurationBlock.textareaCallback(e);
    if ((code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) && open)
        self->showKeyboard(lv_event_get_current_target(e));
    self->unlock();
}
