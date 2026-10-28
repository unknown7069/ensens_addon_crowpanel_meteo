#include "TimestampBlock.h"
#include "entities/ui/components/common.h"

namespace
{
// Newest-to-oldest year options for the calendar header dropdown.
const char* const kCalendarYearList =
    "2050\n2049\n2048\n2047\n2046\n2045\n2044\n2043\n2042\n2041\n2040"
    "\n2039\n2038\n2037\n2036\n2035\n2034\n2033\n2032\n2031\n2030"
    "\n2029\n2028\n2027\n2026\n2025";
} // namespace

void TimestampBlock::create(Menu& menu)
{
    page.create(menu, "Date & time");
    lv_obj_t* item = page.createSidebarItem("Date & time");
    const auto date = BM8563::instance().getDate();
    const auto view = settings_time_create(page.getPage(), date.year, date.month, date.day);
    hour_label_ = view.values[0];
    min_label_ = view.values[1];
    sec_label_ = view.values[2];
    lv_calendar_header_dropdown_set_year_list(view.calendar, kCalendarYearList);
    lv_obj_add_event_cb(view.calendar, calendarEventHandler, LV_EVENT_CLICKED, nullptr);
    lv_obj_add_event_cb(item, [](lv_event_t* e) {
        auto* self = static_cast<TimestampBlock*>(lv_event_get_user_data(e));
        self->rtc_time_ = BM8563::instance().getTime();
        self->setTimeLabels(self->rtc_time_);
    }, LV_EVENT_CLICKED, this);

    lv_event_cb_t up[] = {
        [](lv_event_t* e) { static_cast<TimestampBlock*>(lv_event_get_user_data(e))->changeHours(1); },
        [](lv_event_t* e) { static_cast<TimestampBlock*>(lv_event_get_user_data(e))->changeMinutes(1); },
        [](lv_event_t* e) { static_cast<TimestampBlock*>(lv_event_get_user_data(e))->changeSeconds(1); }
    };
    lv_event_cb_t down[] = {
        [](lv_event_t* e) { static_cast<TimestampBlock*>(lv_event_get_user_data(e))->changeHours(-1); },
        [](lv_event_t* e) { static_cast<TimestampBlock*>(lv_event_get_user_data(e))->changeMinutes(-1); },
        [](lv_event_t* e) { static_cast<TimestampBlock*>(lv_event_get_user_data(e))->changeSeconds(-1); }
    };
    for (int i = 0; i < 3; ++i) {
        lv_obj_add_event_cb(view.up[i], up[i], LV_EVENT_CLICKED, this);
        lv_obj_add_event_cb(view.down[i], down[i], LV_EVENT_CLICKED, this);
    }
    lv_obj_add_event_cb(view.apply, [](lv_event_t* e) {
        const auto* self = static_cast<TimestampBlock*>(lv_event_get_user_data(e));
        BM8563::instance().setTime(self->rtc_time_);
    }, LV_EVENT_CLICKED, this);
    rtc_time_ = BM8563::instance().getTime();
    setTimeLabels(rtc_time_);
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
    lv_label_set_text_fmt(hour_label_, "%02u", time.hours);
    lv_label_set_text_fmt(min_label_, "%02u", time.minutes);
    lv_label_set_text_fmt(sec_label_, "%02u", time.seconds);
    lvgl_port_unlock();
}

void TimestampBlock::changeHours(int delta)
{
    int hours = rtc_time_.hours + delta;
    if (hours > 23)
        hours = 0;
    else if (hours < 0)
        hours = 23;
    rtc_time_.hours = static_cast<uint8_t>(hours);
    setTimeLabels(rtc_time_);
}

void TimestampBlock::changeMinutes(int delta)
{
    int minutes = rtc_time_.minutes + delta;
    if (minutes > 59)
    {
        minutes = 0;
        changeHours(1);
    } else if (minutes < 0)
    {
        minutes = 59;
        changeHours(-1);
    }
    rtc_time_.minutes = static_cast<uint8_t>(minutes);
    setTimeLabels(rtc_time_);
}

void TimestampBlock::changeSeconds(int delta)
{
    int seconds = rtc_time_.seconds + delta;
    if (seconds > 59)
    {
        seconds = 0;
        changeMinutes(1);
    } else if (seconds < 0)
    {
        seconds = 59;
        changeMinutes(-1);
    }
    rtc_time_.seconds = static_cast<uint8_t>(seconds);
    setTimeLabels(rtc_time_);
}
