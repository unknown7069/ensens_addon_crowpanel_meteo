#include "WIFI.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "string.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_DISCONNECTED_BIT BIT1

void WIFI::eventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId, void* eventData)
{
    WIFI* wifi = reinterpret_cast<WIFI*>(arg);
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    } else if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED)
    {
        if ((wifi->retryNum_ >= 0) && (wifi->retryNum_ < RetryCount))
        {
            esp_wifi_connect();
            wifi->retryNum_++;
            ESP_LOGI(TAG, "Retry to connect to the AP");
        } else if (wifi->retryNum_ == RetryCount)
        {
            xEventGroupSetBits(wifi->eventGroup_, WIFI_DISCONNECTED_BIT);
            wifi->invokeCallbacks(CONNECT_FAIL);
            ESP_LOGI(TAG, "Connect to the AP fail");
        } else
        {
            wifi->invokeCallbacks(DISCONNECTED);
            ESP_LOGI(TAG, "Disconnected from ap");
        }
        xEventGroupClearBits(wifi->eventGroup_, WIFI_CONNECTED_BIT);
        memset(wifi->currentSsid_, 0, sizeof(currentSsid_));
        memset(wifi->currentPass_, 0, sizeof(currentPass_));
        memset(wifi->currentBssid_, 0, sizeof(currentBssid_));
    } else if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*)eventData;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        wifi->retryNum_ = -1;
        xEventGroupSetBits(wifi->eventGroup_, WIFI_CONNECTED_BIT);
        xEventGroupClearBits(wifi->eventGroup_, WIFI_DISCONNECTED_BIT);
        wifi->saveAP(wifi->currentSsid_, wifi->currentBssid_, wifi->currentPass_, wifi->currentAutoconnect_);
        wifi->invokeCallbacks(CONNECTED);
    }
}

void WIFI::invokeCallbacks(Event event)
{
    for (const auto& entry : callbacks_)
    {
        if (entry.function)
            entry.function(event, entry.userData);
    }
}

WIFI::WIFI()
{
    eventGroup_ = xEventGroupCreate();
}

void WIFI::init()
{
    uint8_t wifiStoreSavedVersion = 0;
    if (getWifiStoreVersion(&wifiStoreSavedVersion))
    {
        if (wifiStoreSavedVersion < WifiStoreVersion)
        {
            nvs_handle_t handle;
            if (nvs_open("wifi_store", NVS_READWRITE, &handle) == ESP_OK)
            {
                nvs_iterator_t it;
                if (nvs_entry_find("nvs", "wifi_store", NVS_TYPE_BLOB, &it) == ESP_OK)
                    while (1)
                    {
                        nvs_entry_info_t info;
                        nvs_entry_info(it, &info);
                        nvs_erase_key(handle, info.key);
                        if (nvs_entry_next(&it) != ESP_OK)
                            break;
                    }
                nvs_set_u8(handle, "__version__", WifiStoreVersion);
                nvs_commit(handle);
                nvs_close(handle);
                ESP_LOGI(TAG, "wifi_store reset and version updated to %d", WifiStoreVersion);
            } else
                ESP_LOGE(TAG, "wifi_store reset failed");
        }
        ESP_LOGI(TAG, "wifi_store version: %d", wifiStoreSavedVersion);
    } else
        ESP_LOGE(TAG, "Get wifi_store version failed");
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    esp_wifi_start();

    mutex_ = xSemaphoreCreateMutex();
}

bool WIFI::connectAP(const char* ssid, uint8_t* bssid, const char* pass, bool autoconnect,
                     bool waitForConnect)
{
    xSemaphoreTake(mutex_, portMAX_DELAY);
    esp_wifi_stop();
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &eventHandler,
                                                        this, &handlerAnyId_));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &eventHandler, this, &handlerGotIp_));
    wifi_config_t config = {
        .sta =
            {
                .threshold = {.authmode = WIFI_AUTH_OPEN},
                .sae_pwe_h2e = WPA3_SAE_PWE_HUNT_AND_PECK,
                .sae_h2e_identifier = "",
            },
    };
    retryNum_ = 0;
    memcpy(config.sta.ssid, ssid, strnlen(ssid, 32) + 1);
    memcpy(config.sta.password, pass, strnlen(pass, 64) + 1);
    strncpy(currentSsid_, ssid, sizeof(currentSsid_));
    memcpy(currentBssid_, bssid, sizeof(currentBssid_));
    strncpy(currentPass_, pass, sizeof(currentPass_));
    currentAutoconnect_ = autoconnect;
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "connect() ssid = %s", ssid);

    xSemaphoreGive(mutex_);

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    if (waitForConnect)
    {
        EventBits_t bits =
            xEventGroupWaitBits(eventGroup_, WIFI_CONNECTED_BIT | WIFI_DISCONNECTED_BIT, pdFALSE,
                                pdFALSE, portMAX_DELAY);
        if (bits & WIFI_CONNECTED_BIT)
        {
            ESP_LOGD(TAG, "connected to ap SSID:%s", ssid);
            return true;
        } else if (bits & WIFI_DISCONNECTED_BIT)
        {
            ESP_LOGD(TAG, "Failed to connect to SSID:%s", ssid);
            esp_wifi_stop();
        } else
            ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
    return false;
}

void WIFI::waitForConnection()
{
    xEventGroupWaitBits(eventGroup_, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

bool WIFI::isConnected()
{
    auto bits = xEventGroupWaitBits(eventGroup_, WIFI_CONNECTED_BIT | WIFI_DISCONNECTED_BIT, pdFALSE,
                                    pdFALSE, 0);
    return bits & WIFI_CONNECTED_BIT ? true : false;
}

bool WIFI::getCurrentAP(char* ssid, int8_t* rssi)
{
    if (!ssid || !rssi)
        return false;
    static wifi_ap_record_t ap = {};

    if (esp_wifi_sta_get_ap_info(&ap) != ESP_OK)
    {
        ssid[0] = '\0';
        return false;
    }

    memcpy(ssid, ap.ssid, strnlen((char*)ap.ssid, 32) + 1);
    *rssi = ap.rssi;
    return true;
}

bool WIFI::disconnect()
{
    bool retVal = true;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    if ((esp_wifi_disconnect() != ESP_OK) && ((esp_wifi_stop() != ESP_OK)))
        retVal = false;
    xSemaphoreGive(mutex_);
    return retVal;
}

bool WIFI::scan()
{
    bool retVal = true;
    xSemaphoreTake(mutex_, portMAX_DELAY);
    wifi_scan_config_t scan_config = { .ssid        = 0,
                                       .bssid       = 0,
                                       .channel     = 0,
                                       .show_hidden = true,
                                       .scan_type   = WIFI_SCAN_TYPE_ACTIVE,
                                       .scan_time   = { .active = { .min = 100, .max = 300 } } };

    if (esp_wifi_scan_start(&scan_config, true) != ESP_OK)
    {
        retVal = false;
        ESP_LOGE(TAG, "esp_wifi_scan_start ()");
    }
    if ((esp_wifi_scan_get_ap_num(&scannedApCount_) != ESP_OK) || (scannedApCount_ == 0) ||
        ((scannedAPs_ = reinterpret_cast<wifi_ap_record_t*>(
              realloc(scannedAPs_, (sizeof(wifi_ap_record_t) * scannedApCount_)))) == nullptr) ||
        (esp_wifi_scan_get_ap_records(&scannedApCount_, scannedAPs_) != ESP_OK) ||
        (esp_wifi_scan_stop() != ESP_OK))
    {
        retVal = false;
        ESP_LOGE(TAG, "scan() stop failed");
    }
    xSemaphoreGive(mutex_);
    return retVal;
}

bool WIFI::getScannedAP(wifi_ap_record_t** accessPoints, uint16_t* count)
{
    bool retVal = true;
    xSemaphoreTake(mutex_, portMAX_DELAY);

    *accessPoints = scannedAPs_;
    *count        = scannedApCount_;

    xSemaphoreGive(mutex_);
    return retVal;
}

static void makeNvsKey(char* outKey, size_t len, const uint8_t* bssid)
{
    snprintf(outKey, len, "ap_%02X%02X%02X%02X%02X%02X", bssid[0], bssid[1], bssid[2], bssid[3],
             bssid[4], bssid[5]);
}

bool WIFI::saveAP(const char* ssid, const uint8_t* bssid, const char* pass, bool autoconnect)
{
    if (!ssid || !bssid || !pass)
        return false;

    APInfo       ap = {};
    char         key[NVS_KEY_NAME_MAX_SIZE];
    nvs_handle_t handle;
    bool         retVal = true;
    strncpy(ap.ssid, ssid, sizeof(ap.ssid) - 1);
    strncpy(ap.pass, pass, sizeof(ap.pass) - 1);
    memcpy(ap.bssid, bssid, 6);
    ap.auto_connect = autoconnect;

    makeNvsKey(key, sizeof(key), bssid);

    if ((nvs_open("wifi_store", NVS_READWRITE, &handle) != ESP_OK) ||
        (nvs_set_blob(handle, key, &ap, sizeof(APInfo)) != ESP_OK) ||
        (nvs_commit(handle) != ESP_OK))
        retVal = false;

    nvs_close(handle);

    if (!retVal)
        ESP_LOGE(TAG, "Failed to save AP: %s", ssid);
    else
        ESP_LOGI(TAG,
                 "Saved AP: %s (key: %s, autoconnect -%d) "
                 "%02X:%02X:%02X:%02X:%02X:%02X",
                 ssid, key, autoconnect, bssid[0], bssid[1], bssid[2], bssid[3], bssid[4],
                 bssid[5]);

    return retVal;
}

bool WIFI::getAP(const char* ssid, const uint8_t* bssid, char* pass, bool* autoconnect)
{
    if (!ssid || !bssid || !pass || !autoconnect)
        return false;

    char         key[NVS_KEY_NAME_MAX_SIZE];
    nvs_handle_t handle;
    APInfo       ap     = {};
    size_t       len    = sizeof(APInfo);
    bool         retVal = true;
    makeNvsKey(key, sizeof(key), bssid);

    if ((nvs_open("wifi_store", NVS_READWRITE, &handle) != ESP_OK) ||
        (nvs_get_blob(handle, key, &ap, &len) != ESP_OK))
        retVal = false;

    nvs_close(handle);

    if (!retVal || (strncmp(ap.ssid, ssid, sizeof(ap.ssid)) != 0))
        return false;

    strncpy(pass, ap.pass, MAX_PASSPHRASE_LEN);
    *autoconnect = ap.auto_connect;
    return true;
}

bool WIFI::eraseAP(const char* ssid, const uint8_t* bssid)
{
    if (!ssid || !bssid)
        return false;

    char         key[NVS_KEY_NAME_MAX_SIZE];
    nvs_handle_t handle;
    bool         retVal = true;
    makeNvsKey(key, sizeof(key), bssid);

    if ((nvs_open("wifi_store", NVS_READWRITE, &handle) != ESP_OK) ||
        (nvs_erase_key(handle, key) != ESP_OK) || (nvs_commit(handle) != ESP_OK))
        retVal = false;

    nvs_close(handle);

    if (!retVal)
        ESP_LOGE(TAG, "Failed to remove AP: %s", ssid);
    else
        ESP_LOGI(TAG, "Removed AP: %s (key: %s)", ssid, key);

    return retVal;
}

bool WIFI::getSavedAPs(std::vector<APInfo>& list)
{
    nvs_handle_t handle;
    if (nvs_open("wifi_store", NVS_READONLY, &handle) != ESP_OK)
        return false;

    nvs_iterator_t it;
    if (nvs_entry_find("nvs", "wifi_store", NVS_TYPE_BLOB, &it) != ESP_OK)
    {
        nvs_close(handle);
        return false;
    }

    while (1)
    {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);

        if (strncmp(info.key, "ap_", 3) == 0)
        {
            APInfo ap  = {};
            size_t len = sizeof(APInfo);
            if (nvs_get_blob(handle, info.key, &ap, &len) == ESP_OK)
                list.push_back(ap);
            if (len != sizeof(APInfo))
                ESP_LOGW(TAG, "Skipping NVS entry %s: size mismatch (%d != %d)", info.key, len,
                         sizeof(APInfo));
        }

        if (nvs_entry_next(&it) != ESP_OK)
            break;
    }

    nvs_release_iterator(it);
    nvs_close(handle);
    return !list.empty();
}

bool WIFI::addCallback(Callback callback, void* userData)
{
    if (callback == nullptr)
        return false;

    callbacks_.push_back({ callback, userData });
    return true;
}

bool WIFI::getWifiStoreVersion(uint8_t* version)
{
    if (!version)
        return false;

    nvs_handle_t handle;

    if (nvs_open("wifi_store", NVS_READWRITE, &handle) != ESP_OK)
        return false;

    esp_err_t err = nvs_get_u8(handle, "__version__", version);
    nvs_close(handle);

    if (err == ESP_ERR_NVS_NOT_FOUND)
        *version = 0;
    else if (err != ESP_OK)
        return false;

    return true;
}