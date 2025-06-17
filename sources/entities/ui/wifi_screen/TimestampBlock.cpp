#include "TimestampBlock.h"
#include "entities/ui/components/common.h"

void TimestampBlock::create(Menu& menu)
{
    page.create(menu, "Manual date and time configuration");
    lv_obj_t* item = page.createSidebarItem("Date and time");

    /// Spacer.
    spacer.create(page.getPage(), LV_PCT(100), 10, LV_FLEX_FLOW_ROW);

    lv_obj_add_event_cb(
        item,
        [](lv_event_t* e) {
            auto* self      = static_cast<TimestampBlock*>(lv_event_get_user_data(e));
            self->rtc_time_ = BM8563::instance().getTime();
            self->setTimeLabels(self->rtc_time_);
        },
        LV_EVENT_CLICKED, this);

    manual_date_time_container_ = lv_obj_create(page.getPage());
    lv_obj_set_flex_flow(manual_date_time_container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(manual_date_time_container_, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(manual_date_time_container_, 1, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(manual_date_time_container_, 1, LV_PART_MAIN);
    lv_obj_set_size(manual_date_time_container_, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_add_style(manual_date_time_container_, &transparent_area_style, LV_PART_MAIN);

    // Calendar
    lv_obj_t* calendar = lv_calendar_create(manual_date_time_container_);
    lv_obj_add_event_cb(calendar, calendarEventHandler, LV_EVENT_ALL, nullptr);
    lv_calendar_header_dropdown_create(calendar);
    lv_obj_set_size(calendar, lv_pct(100), LV_SIZE_CONTENT);
    static auto year_list = "2050\n2049\n2048\n2047\n2046\n2045\n2044\n2043\n2042\n2041\n2040"
                            "\n2039\n2038\n2037\n2036\n2035\n2034\n2033\n2032\n2031\n2030"
                            "\n2029\n2028\n2027\n2026\n2025";
    lv_calendar_header_dropdown_set_year_list(calendar, year_list);

    const BM8563::Date_t date = BM8563::instance().getDate();
    lv_calendar_set_today_date(calendar, date.year, date.month, date.day);
    lv_calendar_set_showed_date(calendar, date.year, date.month);

    // Time
    lv_obj_t* manual_time_container = lv_obj_create(manual_date_time_container_);
    lv_obj_set_flex_flow(manual_time_container, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(manual_time_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_size(manual_time_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_add_style(manual_time_container, &transparent_area_style, LV_PART_MAIN);

    // Hours
    lv_obj_t* hour_col = lv_obj_create(manual_time_container);
    lv_obj_clear_flag(hour_col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_align(hour_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_flow(hour_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(hour_col, 1);
    lv_obj_set_height(hour_col, LV_SIZE_CONTENT);
    lv_obj_add_style(hour_col, &transparent_area_style, LV_PART_MAIN);

    lv_obj_t* hour_up_btn   = lv_btn_create(hour_col);
    lv_obj_t* hour_up_label = lv_label_create(hour_up_btn);
    lv_label_set_text(hour_up_label, LV_SYMBOL_UP);
    lv_obj_center(hour_up_label);
    lv_obj_add_event_cb(
        hour_up_btn,
        [](lv_event_t* e) {
            auto* self = static_cast<TimestampBlock*>(lv_event_get_user_data(e));
            self->increaseHours();
        },
        LV_EVENT_CLICKED, this);

    hour_label_ = lv_label_create(hour_col);

    lv_obj_t* hour_down_btn   = lv_btn_create(hour_col);
    lv_obj_t* hour_down_label = lv_label_create(hour_down_btn);
    lv_label_set_text(hour_down_label, LV_SYMBOL_DOWN);
    lv_obj_center(hour_down_label);
    lv_obj_add_event_cb(
        hour_down_btn,
        [](lv_event_t* e) {
            auto* self = static_cast<TimestampBlock*>(lv_event_get_user_data(e));
            self->decreaseHours();
        },
        LV_EVENT_CLICKED, this);

    // Minutes
    lv_obj_t* min_col = lv_obj_create(manual_time_container);
    lv_obj_clear_flag(min_col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_align(min_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_flow(min_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(min_col, 1);
    lv_obj_set_height(min_col, LV_SIZE_CONTENT);
    lv_obj_add_style(min_col, &transparent_area_style, LV_PART_MAIN);

    lv_obj_t* min_up_btn   = lv_btn_create(min_col);
    lv_obj_t* min_up_label = lv_label_create(min_up_btn);
    lv_label_set_text(min_up_label, LV_SYMBOL_UP);
    lv_obj_center(min_up_label);
    lv_obj_add_event_cb(
        min_up_btn,
        [](lv_event_t* e) {
            auto* self = static_cast<TimestampBlock*>(lv_event_get_user_data(e));
            self->increaseMinutes();
        },
        LV_EVENT_CLICKED, this);

    min_label_ = lv_label_create(min_col);

    lv_obj_t* min_down_btn   = lv_btn_create(min_col);
    lv_obj_t* min_down_label = lv_label_create(min_down_btn);
    lv_label_set_text(min_down_label, LV_SYMBOL_DOWN);
    lv_obj_center(min_down_label);
    lv_obj_add_event_cb(
        min_down_btn,
        [](lv_event_t* e) {
            auto* self = static_cast<TimestampBlock*>(lv_event_get_user_data(e));
            self->decreaseMinutes();
        },
        LV_EVENT_CLICKED, this);

    // Seconds
    lv_obj_t* sec_col = lv_obj_create(manual_time_container);
    lv_obj_clear_flag(sec_col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_align(sec_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_flow(sec_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(sec_col, 1);
    lv_obj_set_height(sec_col, LV_SIZE_CONTENT);
    lv_obj_add_style(sec_col, &transparent_area_style, LV_PART_MAIN);

    lv_obj_t* sec_up_btn   = lv_btn_create(sec_col);
    lv_obj_t* sec_up_label = lv_label_create(sec_up_btn);
    lv_label_set_text(sec_up_label, LV_SYMBOL_UP);
    lv_obj_center(sec_up_label);
    lv_obj_add_event_cb(
        sec_up_btn,
        [](lv_event_t* e) {
            auto* self = static_cast<TimestampBlock*>(lv_event_get_user_data(e));
            self->increaseSeconds();
        },
        LV_EVENT_CLICKED, this);

    sec_label_ = lv_label_create(sec_col);

    lv_obj_t* sec_down_btn   = lv_btn_create(sec_col);
    lv_obj_t* sec_down_label = lv_label_create(sec_down_btn);
    lv_label_set_text(sec_down_label, LV_SYMBOL_DOWN);
    lv_obj_center(sec_down_label);
    lv_obj_add_event_cb(
        sec_down_btn,
        [](lv_event_t* e) {
            auto* self = static_cast<TimestampBlock*>(lv_event_get_user_data(e));
            self->decreaseSeconds();
        },
        LV_EVENT_CLICKED, this);

    // Confirm
    lv_obj_t* cancel_confirm_col = lv_obj_create(manual_time_container);
    lv_obj_clear_flag(cancel_confirm_col, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_align(cancel_confirm_col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_SPACE_BETWEEN);
    lv_obj_set_flex_flow(cancel_confirm_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_grow(cancel_confirm_col, 1);
    lv_obj_set_height(cancel_confirm_col, LV_SIZE_CONTENT);
    lv_obj_add_style(cancel_confirm_col, &transparent_area_style, LV_PART_MAIN);

    lv_obj_t* confirm_btn   = lv_btn_create(cancel_confirm_col);
    lv_obj_t* confirm_label = lv_label_create(confirm_btn);
    lv_label_set_text(confirm_label, "Confirm");
    lv_obj_center(confirm_label);
    lv_obj_add_event_cb(
        confirm_btn,
        [](lv_event_t* e) {
            const auto* self = static_cast<TimestampBlock*>(lv_event_get_user_data(e));
            BM8563::instance().setTime(self->rtc_time_);
        },
        LV_EVENT_CLICKED, this);
}

void TimestampBlock::calendarEventHandler(lv_event_t* e)
{
    const lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t*             obj  = lv_event_get_current_target(e);

    if (code != LV_EVENT_CLICKED)
        return;

    lv_calendar_date_t date;
    if (lv_calendar_get_pressed_date(obj, &date))
    {
        lv_calendar_set_today_date(obj, date.year, date.month, date.day);
        const BM8563::Date_t new_date = {
            .day     = static_cast<uint8_t>(date.day),
            .weekDay = get_day_of_week(date.year, date.month, date.day),
            .month   = static_cast<uint8_t>(date.month),
            .year    = static_cast<uint16_t>(date.year),
        };
        BM8563::instance().setDate(new_date);
    }
}

void TimestampBlock::setTimeLabels(const BM8563::Time_t time)
{
    lvgl_port_lock();
    lv_label_set_text(hour_label_, (std::to_string(time.hours) + " hours").c_str());
    lv_label_set_text(min_label_, (std::to_string(time.minutes) + " minutes").c_str());
    lv_label_set_text(sec_label_, (std::to_string(time.seconds) + " seconds").c_str());
    lvgl_port_unlock();
}

void TimestampBlock::decreaseHours()
{
    if (rtc_time_.hours > 0)
    {
        rtc_time_.hours--;
    } else
    {
        rtc_time_.hours = 23;
    }
    setTimeLabels(rtc_time_);
}

void TimestampBlock::decreaseMinutes()
{
    if (rtc_time_.minutes > 0)
    {
        rtc_time_.minutes--;
    } else
    {
        rtc_time_.minutes = 59;
        decreaseHours();
    }
    setTimeLabels(rtc_time_);
}

void TimestampBlock::decreaseSeconds()
{
    if (rtc_time_.seconds > 0)
    {
        rtc_time_.seconds--;
    } else
    {
        rtc_time_.seconds = 59;
        decreaseMinutes();
    }
    setTimeLabels(rtc_time_);
}

void TimestampBlock::increaseHours()
{
    if (rtc_time_.hours < 23)
    {
        rtc_time_.hours++;
    } else
    {
        rtc_time_.hours = 0;
    }
    setTimeLabels(rtc_time_);
}

void TimestampBlock::increaseMinutes()
{
    if (rtc_time_.minutes < 59)
    {
        rtc_time_.minutes++;
    } else
    {
        rtc_time_.minutes = 0;
        increaseHours();
    }
    setTimeLabels(rtc_time_);
}

void TimestampBlock::increaseSeconds()
{
    if (rtc_time_.seconds < 59)
    {
        rtc_time_.seconds++;
    } else
    {
        rtc_time_.seconds = 0;
        increaseMinutes();
    }
    setTimeLabels(rtc_time_);
}