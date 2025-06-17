#pragma once

#include "adapters/lvgl/LabelBase.h"

class TimeZoneLabel : LabelBase
{
    static constexpr uint8_t labelTextLength = 30;
    char                     text[labelTextLength];

public:
    void create(lv_obj_t* parent, const lv_font_t* font, lv_align_t align = LV_ALIGN_CENTER,
                lv_coord_t xOffs = 0, lv_coord_t yOffs = 0)
    {
        lock();
        LabelBase::create(parent, text, sizeof(text), align, xOffs, yOffs, font, false);
        appendText("Loading: ...", true);
        unlock();
    }

    void setCurrentTime(uint32_t timestamp)
    {
        lock();
        clean(false);
        if (timestamp != 0)
        {
            time_t     timestampStruct = timestamp;
            struct tm* timeInfo        = localtime(&timestampStruct);
            strftime(text, sizeof(text), "%H:%M", timeInfo);
        } else
            text[0] = '\0';
        updateOnScreen();
        unlock();
    }

    void setCurrentDate(uint32_t timestamp, const char* format = "%d.%m.%y, %a")
    {
        lock();
        clean(false);
        if (timestamp != 0)
        {
            time_t     timestampStruct = timestamp;
            struct tm* timeInfo        = localtime(&timestampStruct);
            strftime(text, sizeof(text), format, timeInfo);
        } else
            text[0] = '\0';
        updateOnScreen();
        unlock();
    }

    lv_obj_t* get()
    {
        return LabelBase::get();
    }
};