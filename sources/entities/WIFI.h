#pragma once

#include "freertos/FreeRTOS.h"
#include "nvs_flash.h"
#include <array>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <vector>

class WIFI
{
public:
    struct APInfo {
        char    ssid[MAX_SSID_LEN + 1];
        char    pass[MAX_PASSPHRASE_LEN];
        uint8_t bssid[6];
        bool    auto_connect;
    };
    enum Event
    {
        CONNECTED,
        CONNECT_FAIL,
        DISCONNECTED
    };
    using Callback = void(Event, void*);
    struct CallbackEntry {
        Callback* function;
        void*     userData;
    };
    WIFI();
    static WIFI& instance()
    {
        static WIFI instance;
        return instance;
    }
    void init();
    bool connectAP(const char* ssid, uint8_t* bssid, const char* pass, bool autoconnect,
                   bool waitForConnect = false);
    bool isConnected();
    bool getCurrentAP(char* ssid, int8_t* rssi);
    bool disconnect();
    bool scan();
    bool getScannedAP(wifi_ap_record_t** accessPoints, uint16_t* count);

    bool saveAP(const char* ssid, const uint8_t* bssid, const char* pass, bool autoconnect);
    bool eraseAP(const char* ssid, const uint8_t* bssid);
    bool getAP(const char* ssid, const uint8_t* bssid, char* pass, bool* autoconnect);
    bool getSavedAPs(std::vector<APInfo>& list);
    bool addCallback(Callback callback, void* userData);
    void invokeCallbacks(Event event);
    void waitForConnection();

private:
    static constexpr char    TAG[]            = "wifi";
    static constexpr uint8_t RetryCount       = 1;
    static constexpr uint8_t WifiStoreVersion = 3;

    EventGroupHandle_t           eventGroup_;
    int                          retryNum_ = 0;
    esp_event_handler_instance_t handlerAnyId_;
    esp_event_handler_instance_t handlerGotIp_;
    static void                eventHandler(void* arg, esp_event_base_t eventBase, int32_t eventId,
                                            void* eventData);
    SemaphoreHandle_t          mutex_;
    wifi_ap_record_t*          scannedAPs_;
    uint16_t                   scannedApCount_;
    char                       currentSsid_[MAX_SSID_LEN + 1];
    char                       currentPass_[MAX_PASSPHRASE_LEN];
    uint8_t                    currentBssid_[6];
    bool                       currentAutoconnect_;
    std::vector<CallbackEntry> callbacks_;

    bool getWifiStoreVersion(uint8_t* version);
};