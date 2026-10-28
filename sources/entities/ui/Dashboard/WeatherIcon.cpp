#include "WeatherIcon.h"
#include "KitchenTheme.h"

#include <algorithm>
#include <cstdint>
#include <cstring>

namespace {
struct Painter {
    lv_draw_ctx_t* ctx;
    lv_area_t bounds;
    int size;

    lv_coord_t x(int v) const { return bounds.x1 + v * size / 100; }
    lv_coord_t y(int v) const { return bounds.y1 + v * size / 100; }

    void rectangle(int x1, int y1, int x2, int y2, lv_color_t color, bool round = false)
    {
        lv_draw_rect_dsc_t d;
        lv_draw_rect_dsc_init(&d);
        d.bg_color = color;
        d.bg_opa = LV_OPA_COVER;
        d.radius = round ? LV_RADIUS_CIRCLE : 0;
        lv_area_t a = {x(x1), y(y1), x(x2), y(y2)};
        lv_draw_rect(ctx, &d, &a);
    }

    void circle(int cx, int cy, int radius, lv_color_t color)
    {
        rectangle(cx - radius, cy - radius, cx + radius, cy + radius, color, true);
    }

    void line(int x1, int y1, int x2, int y2, lv_color_t color, int width = 4)
    {
        lv_draw_line_dsc_t d;
        lv_draw_line_dsc_init(&d);
        d.color = color;
        d.width = std::max(1, width * size / 100);
        d.round_start = d.round_end = true;
        lv_point_t a = {x(x1), y(y1)}, b = {x(x2), y(y2)};
        lv_draw_line(ctx, &d, &a, &b);
    }

    void sun(int cx, int cy, int r)
    {
        const auto color = lv_color_hex(KitchenTheme::Terracotta);
        circle(cx, cy, r, color);
        for (int a = 0; a < 360; a += 45)
        {
            const int sx = lv_trigo_sin(a), sy = lv_trigo_sin(a + 90);
            line(cx + sx * (r + 7) / 32768, cy + sy * (r + 7) / 32768,
                 cx + sx * (r + 15) / 32768, cy + sy * (r + 15) / 32768, color);
        }
    }

    void moon(lv_color_t background)
    {
        circle(44, 42, 29, lv_color_hex(KitchenTheme::Slate));
        circle(58, 30, 27, background);
    }

    void cloud(lv_color_t background)
    {
        const auto ink = lv_color_hex(KitchenTheme::Ink);
        // Draw the union's outline, then its inset fill; no internal seams.
        circle(29, 63, 17, ink);
        circle(50, 51, 24, ink);
        circle(75, 63, 17, ink);
        rectangle(29, 62, 75, 80, ink);
        circle(29, 63, 13, background);
        circle(50, 51, 20, background);
        circle(75, 63, 13, background);
        rectangle(29, 59, 75, 76, background);
    }
};

void drawIcon(lv_event_t* e)
{
    lv_obj_t* obj = lv_event_get_target(e);
    Painter p{lv_event_get_draw_ctx(e), {}, lv_obj_get_width(obj)};
    lv_obj_get_coords(obj, &p.bounds);
    const uintptr_t code = reinterpret_cast<uintptr_t>(lv_obj_get_user_data(obj));
    const int condition = code & 0xff;
    const bool night = code & 0x100;
    const auto background = lv_obj_get_style_bg_color(obj, LV_PART_MAIN);
    const auto slate = lv_color_hex(KitchenTheme::Slate);
    if (condition == 1 || condition == 2)
    {
        if (night) p.moon(background);
        else p.sun(condition == 1 ? 50 : 34, condition == 1 ? 50 : 32,
                   condition == 1 ? 22 : 17);
    }
    if (condition == 2 || condition == 3 || condition == 4 ||
        condition == 9 || condition == 10 || condition == 11 || condition == 13)
        p.cloud(background);
    if (condition == 9 || condition == 10)
        for (int x = 30; x <= 75; x += 20)
            p.line(x, 87, x - 4, 96, slate);
    if (condition == 11)
    {
        const auto amber = lv_color_hex(KitchenTheme::Terracotta);
        p.line(55, 80, 45, 89, amber, 6);
        p.line(45, 89, 55, 89, amber, 6);
        p.line(55, 89, 45, 98, amber, 6);
    }
    if (condition == 13)
        for (int x = 30; x <= 75; x += 20)
        {
            p.line(x - 4, 91, x + 4, 91, slate, 3);
            p.line(x, 87, x, 95, slate, 3);
        }
    if (condition == 50)
        for (int y = 35; y <= 70; y += 15)
            p.line(y == 50 ? 15 : 25, y, y == 50 ? 85 : 75, y, slate);
    if (!condition)
        p.line(38, 50, 62, 50, lv_color_hex(KitchenTheme::Rule));
}
}

lv_obj_t* weather_icon_create(lv_obj_t* parent, lv_coord_t size, uint32_t background)
{
    lv_obj_t* obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, size, size);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_color(obj, lv_color_hex(background), 0);
    lv_obj_set_user_data(obj, nullptr);
    lv_obj_add_event_cb(obj, drawIcon, LV_EVENT_DRAW_MAIN, nullptr);
    return obj;
}

void weather_icon_set(lv_obj_t* icon, const char* openWeatherCode)
{
    uintptr_t code = 0;
    if (openWeatherCode && std::strlen(openWeatherCode) == 3 &&
        (openWeatherCode[2] == 'd' || openWeatherCode[2] == 'n'))
    {
        const int number = (openWeatherCode[0] - '0') * 10 + openWeatherCode[1] - '0';
        if (number == 1 || number == 2 || number == 3 || number == 4 ||
            number == 9 || number == 10 || number == 11 || number == 13 || number == 50)
            code = number | (openWeatherCode[2] == 'n' ? 0x100 : 0);
    }
    lv_obj_set_user_data(icon, reinterpret_cast<void*>(code));
    lv_obj_invalidate(icon);
}
