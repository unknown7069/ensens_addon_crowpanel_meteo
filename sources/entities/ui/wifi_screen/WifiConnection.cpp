#include "WifiConnection.h"
#include "WifiScreen.h"
#include "entities/ui/Dashboard/KitchenTheme.h"

LV_IMG_DECLARE(wifi_100);
LV_IMG_DECLARE(wifi_75);
LV_IMG_DECLARE(wifi_50);
LV_IMG_DECLARE(wifi_25);
LV_IMG_DECLARE(settings);

void WifiConnection::create(Menu& menu, lv_obj_t** _keyboard)
{
    page.create(menu, "Wi-Fi");
    page.createSidebarItem("Wi-Fi");

    /// Spacer.
    spacer.create(page.getPage(), LV_PCT(100), 1, LV_FLEX_FLOW_ROW);
    {
        connectedListContainer.create(page.getPage(), LV_PCT(100), LV_SIZE_CONTENT,
                                      LV_FLEX_FLOW_COLUMN);
        connectedListContainer.align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

        availableListContainer.create(page.getPage(), LV_PCT(100), LV_SIZE_CONTENT,
                                      LV_FLEX_FLOW_COLUMN);
        availableListContainer.align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        {
            availableLabel.create(availableListContainer.get(), &lv_font_montserrat_22);
            availableLabel.align(LV_ALIGN_TOP_LEFT);
            availableLabel.setText("Available networks");
        }
    }
    availableListContainer.paddingGap(10);
    keyboardSpacer.create(page.getPage(), LV_PCT(100), 1,
                          LV_FLEX_FLOW_COLUMN);
    keyboard = _keyboard;

    WIFI::instance().addCallback(wifiEventHandler, this);
}

void WifiConnection::wifiEventHandler(WIFI::Event event, void* context)
{
    WifiConnection* screen = (WifiConnection*)context;
    lvgl_port_lock();
    switch (event)
    {
    case WIFI::Event::CONNECTED:
        if (screen->currentAP)
            screen->currentAP->remove();
        ESP_LOGI(TAG, "wifi connected, AP removed");
        break;
    case WIFI::Event::DISCONNECTED:
        if (screen->currentAP)
            screen->currentAP->show(screen->availableListContainer.get());
        break;
    case WIFI::Event::CONNECT_FAIL:
        if (screen->currentAP)
            screen->currentAP->deselect();
        break;
    default:
        break;
    }
    ESP_LOGI(TAG, "wifi event - %d", event);
    char   ssid[MAX_SSID_LEN + 1] = { 0 };
    int8_t rssi                   = 0;
    WIFI::instance().getCurrentAP(ssid, &rssi);
    screen->updateCurrentSSID(ssid, rssi);
    lvgl_port_unlock();
}

void WifiConnection::passwordKeyboardEventHandler()
{
    WifiConnection* screen                       = WifiScreen::instance().wifiConnectionBlock;
    char            password[MAX_PASSPHRASE_LEN] = { 0 };
    strncpy(password, lv_textarea_get_text(screen->textAreaPassword), MAX_PASSPHRASE_LEN);
    if (screen->textAreaPassword)
    {
        lv_obj_remove_event_cb(screen->textAreaPassword,
                               WifiScreen::instance().textareaEventCallback);
        lv_obj_del(screen->textAreaPassword);
        screen->textAreaPassword = nullptr;
    }
    bool auto_connect = lv_obj_get_state(screen->checkboxAutoConnect) & LV_STATE_CHECKED;
    if (screen->checkboxAutoConnect)
    {
        lv_obj_del(screen->checkboxAutoConnect);
        screen->checkboxAutoConnect = nullptr;
    }

    screen->connect(screen->currentAP, password, auto_connect);
}

void WifiConnection::updateCurrentSSID(char* ssid, int8_t rssi)
{
    static bool connected = false;

    if ((!ssid) || (ssid[0] == '\0'))
    {
        connectedLabel.remove();
        spaceContainer.remove();
        currentWifiIcon.remove();
        currentWifiLabel.remove();
        currentInfoContainer.remove();
        disconnectLabel.remove();
        disconnect.remove();
        currentDisconnectContainer.remove();
        currentItemContainer.remove();
        connected = false;
        return;
    }

    if (!connected)
    {
        connected = true;

        // connected label
        connectedLabel.create(connectedListContainer.get(), &lv_font_montserrat_22);
        connectedLabel.align(LV_ALIGN_TOP_LEFT);
        connectedLabel.setText("Connected network");
        spaceContainer.create(connectedListContainer.get(), LV_PCT(100), 10, LV_FLEX_FLOW_ROW);
        // wifiAPInfoContainer (row)
        currentItemContainer.create(connectedListContainer.get(), LV_PCT(100), LV_SIZE_CONTENT,
                                    LV_FLEX_FLOW_ROW);
        currentItemContainer.align(LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START);
        currentItemContainer.padding(10, 0);
        {
            currentInfoContainer.create(currentItemContainer.get(), LV_PCT(50), LV_SIZE_CONTENT,
                                        LV_FLEX_FLOW_ROW);
            currentInfoContainer.align(LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                                       LV_FLEX_ALIGN_CENTER);
            currentInfoContainer.paddingGap(10);
            // wifi icon
            currentWifiIcon.create(currentInfoContainer.get());

            // wifi label
            currentWifiLabel.create(currentInfoContainer.get(), &lv_font_montserrat_22);
            lv_obj_set_flex_grow(currentWifiLabel.get(), 1);
            lv_label_set_long_mode(currentWifiLabel.get(), LV_LABEL_LONG_DOT);
            currentWifiLabel.align(LV_ALIGN_TOP_LEFT);
            // disconnect button
            currentDisconnectContainer.create(currentItemContainer.get(), LV_PCT(50),
                                              LV_SIZE_CONTENT, LV_FLEX_FLOW_ROW);
            currentDisconnectContainer.align(LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER,
                                             LV_FLEX_ALIGN_CENTER);

            disconnect.create(currentDisconnectContainer.get(), this);
            lv_obj_set_style_pad_all(disconnect.get(), 20, LV_PART_MAIN);
            lv_obj_set_style_border_width(disconnect.get(), 1, LV_PART_MAIN);
            lv_obj_set_style_border_color(disconnect.get(), lv_color_hex(KitchenTheme::Rule), LV_PART_MAIN);
            disconnect.setEventCallback(disconnectButtonCallback);
            lv_obj_align(disconnect.get(), LV_ALIGN_RIGHT_MID, 0, 0);
            disconnectLabel.create(disconnect.get(), &lv_font_montserrat_22);
            disconnectLabel.setText("Disconnect");
            lv_obj_set_style_img_recolor(currentWifiIcon.get(), lv_color_hex(KitchenTheme::Ink), 0);
            lv_obj_set_style_img_recolor_opa(currentWifiIcon.get(), LV_OPA_COVER, 0);
        }
    }

    if (rssi >= -50)
        currentWifiIcon.set(&wifi_100);
    else if (rssi >= -60)
        currentWifiIcon.set(&wifi_75);
    else if (rssi >= -70)
        currentWifiIcon.set(&wifi_50);
    else
        currentWifiIcon.set(&wifi_25);
    currentWifiLabel.setText(ssid);
}

void WifiConnection::disconnectButtonCallback(lv_event_t* e, void* context)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        lvgl_port_lock();
        WIFI::instance().disconnect();
        char ssid = '\0';
        ESP_LOGI(TAG, "Disconnecting");
        (static_cast<WifiConnection*>(context))->updateCurrentSSID(&ssid, 0);
        lvgl_port_unlock();
    }
}

void WifiConnection::connectButtonCallback(lv_event_t* e, void* _context)
{
    WifiScreen*     screen = &WifiScreen::instance();
    WifiConnection* block  = WifiScreen::instance().wifiConnectionBlock;
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
        return;

    if (!_context)
        return;

    AccessPointItem* net = (AccessPointItem*)_context;

    if (WIFI::instance().isConnected())
        return;

    block->currentAP = net;

    char password[MAX_PASSPHRASE_LEN];
    bool auto_connect;
    if (WIFI::instance().getAP(net->ssid, net->bssid, password, &auto_connect))
    {
        lvgl_port_lock();
        ESP_LOGI(TAG, "connectButton AP found lock");
        block->connect(net, password, auto_connect);
        lvgl_port_unlock();
        return;
    }

    lvgl_port_lock();
    if (block->textAreaPassword)
    {
        lv_obj_remove_event_cb(block->textAreaPassword, screen->textareaEventCallback);
        lv_obj_del(block->textAreaPassword);
        block->textAreaPassword = nullptr;
    }
    block->textAreaPassword = lv_textarea_create(net->dataContainer.get());
    lv_textarea_set_text(block->textAreaPassword, "");
    lv_textarea_set_password_mode(block->textAreaPassword, true);
    lv_textarea_set_one_line(block->textAreaPassword, true);
    lv_obj_set_width(block->textAreaPassword, lv_pct(100));
    lv_obj_add_event_cb(block->textAreaPassword, screen->textareaEventCallback, LV_EVENT_ALL, NULL);

    if (block->checkboxAutoConnect)
    {
        lv_obj_del(block->checkboxAutoConnect);
        block->checkboxAutoConnect = nullptr;
    }
    block->checkboxAutoConnect = lv_checkbox_create(net->dataContainer.get());
    lv_checkbox_set_text(block->checkboxAutoConnect, "Auto Connect");
    lv_obj_add_state(block->checkboxAutoConnect, LV_STATE_CHECKED);

    screen->showKeyboard(block->textAreaPassword);


    lvgl_port_unlock();
}

void WifiConnection::connect(AccessPointItem* net, char* password, bool auto_connect)
{
    currentAP          = net;
    WifiScreen& screen = WifiScreen::instance();
    ESP_LOGI(TAG, "Connecting to %s (%02X:%02X:%02X:%02X:%02X:%02X), rssi: %d", net->ssid,
             net->bssid[0], net->bssid[1], net->bssid[2], net->bssid[3], net->bssid[4],
             net->bssid[5], net->rssi);
    net->select();
    if (textAreaPassword)
    {
        lv_obj_remove_event_cb(textAreaPassword, screen.textareaEventCallback);
        lv_obj_del(textAreaPassword);
        textAreaPassword = nullptr;
    }
    if (checkboxAutoConnect)
    {
        lv_obj_del(checkboxAutoConnect);
        checkboxAutoConnect = nullptr;
    }
    if (*keyboard)
    {
        screen.hideKeyboard();
    }
    WIFI::instance().connectAP(net->ssid, net->bssid, password, auto_connect);
}

lv_obj_t* WifiConnection::getAvailableAPList()
{
    return availableListContainer.get();
}

lv_obj_t* WifiConnection::getPasswordTextarea()
{
    return textAreaPassword;
}
