#pragma once

#include "freertos/FreeRTOS.h"
#include <esp_event.h>
#include <esp_http_client.h>
#include <esp_log.h>
#include <stdint.h>

class HTTPRequest
{
    static constexpr char     TAG[] = "http-request";
    static constexpr uint32_t kFinishEventBit = 0x01;

    EventGroupHandle_t       eventGroup_ = nullptr;
    char*                    buffer_     = nullptr;
    uint32_t                 bufferLen_   = 0;
    uint32_t                 receivedLen_ = 0;
    char*                    url_         = nullptr;
    esp_http_client_method_t method_;
    const char*              certPem_ = nullptr;

    static esp_err_t httpEventHandler(esp_http_client_event_t* evt)
    {
        HTTPRequest* request = reinterpret_cast<HTTPRequest*>(evt->user_data);
        switch (evt->event_id)
        {
        case HTTP_EVENT_ON_DATA:
            if (request->bufferLen_ < (request->receivedLen_ + evt->data_len))
            {
                ESP_LOGE(TAG, "not enough space");
                break;
            }
            memcpy(request->buffer_ + request->receivedLen_, evt->data, evt->data_len);
            request->receivedLen_ += evt->data_len;
            break;
        case HTTP_EVENT_ON_FINISH: {
            ESP_LOGD(TAG, "Received data (%lu): %s", request->receivedLen_,
                     request->buffer_);

            xEventGroupSetBits(request->eventGroup_, kFinishEventBit);

            break;
        }
        default:
            break;
        }
        return ESP_OK;
    }

public:
    HTTPRequest(char* url, esp_http_client_method_t method, char* buffer, uint32_t bufferLen,
                const char* certPem = nullptr)
        : buffer_{ buffer }, bufferLen_{ bufferLen }, url_{ url }, method_{ method },
          certPem_{ certPem }
    {
        eventGroup_ = xEventGroupCreate();
    }
    ~HTTPRequest()
    {
        vEventGroupDelete(eventGroup_);
    }

    // Performs the request. Returns the number of bytes written to the response
    // buffer, or -1 on failure.
    int32_t perform()
    {
        if (!url_)
            return -1;

        esp_http_client_config_t config = {};
        config.url           = url_;
        config.method        = method_;
        config.event_handler = httpEventHandler;
        config.user_data     = this;
        config.cert_pem      = certPem_;
        receivedLen_ = 0;

        ESP_LOGD(TAG, "starting https request - %s", config.url);

        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == nullptr)
        {
            ESP_LOGE(TAG, "esp_http_client_init failed");
            return -1;
        }
        esp_http_client_set_header(client, "Content-Type", "application/x-www-form-urlencoded");
        esp_err_t status = esp_http_client_perform(client);

        bool finished =
            (status == ESP_OK) &&
            ((xEventGroupWaitBits(eventGroup_, kFinishEventBit, pdFALSE, pdFALSE,
                                  portMAX_DELAY) &
              kFinishEventBit) != 0);
        if (!finished)
            ESP_LOGE(TAG, "esp_http_client_perform err(%d)", status);

        esp_http_client_cleanup(client);
        return finished ? static_cast<int32_t>(receivedLen_) : -1;
    }
};
