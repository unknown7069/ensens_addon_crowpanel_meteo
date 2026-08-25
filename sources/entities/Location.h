#pragma once

#include "esp_heap_caps.h"

class Location
{
    static constexpr const char* TAG      = "Location";
    const uint32_t         buffer_size_ = 2048;
    char*                  response_buffer_ =
        static_cast<char*>(heap_caps_calloc(buffer_size_, sizeof(char), MALLOC_CAP_SPIRAM));

public:
    static constexpr uint8_t MaxCityNameLength = 150;
    struct Data {
        char longitude[20];
        char latitude[20];
        char locationName[MaxCityNameLength];
    };

    static Location& instance()
    {
        static Location instance;
        return instance;
    }

    bool get(Data&);
    bool get(char*);
    bool setManual(char* const locationName);
};