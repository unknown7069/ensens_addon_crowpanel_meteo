#pragma once
#include "adapters/lvgl/Button.h"
#include "adapters/lvgl/ExpandableBlock.h"
#include "adapters/lvgl/FlexContainer.h"
#include "adapters/lvgl/ScreenBase.h"
#include "adapters/lvgl/SimpleLabel.h"
#include "adapters/lvgl/MenuPage.h"
#include "entities/BM8563.h"

class TimestampBlock
{
    MenuPage        page;
    FlexContainer   spacer;
    ExpandableBlock dateTimeConfiguration;
    SimpleLabel     dateTimeHeaderLabel;
    FlexContainer   dateTimeHeaderContainer;
    lv_obj_t*       hour_label_ = nullptr;
    lv_obj_t*       min_label_  = nullptr;
    lv_obj_t*       sec_label_  = nullptr;

    lv_obj_t* manual_date_time_container_ = nullptr;

    BM8563::Time_t rtc_time_ = {};

    void setTimeLabels(BM8563::Time_t time);
    void changeHours(int delta);
    void changeMinutes(int delta);
    void changeSeconds(int delta);
    static void calendarEventHandler(lv_event_t* e);

    // Builds an up-button / value-label / down-button column shared by the
    // hours, minutes and seconds spinners.
    static lv_obj_t* createSpinnerColumn(lv_obj_t* parent, lv_obj_t** value_label,
                                         void (*onUp)(lv_event_t*), void (*onDown)(lv_event_t*),
                                         void* event_user_data);

public:
    void create(Menu&);
};
