#include "common.h"
lv_style_t transparent_area_style;
void ui_styles_init()
{
    lv_style_init(&transparent_area_style);
    lv_style_set_border_opa(&transparent_area_style, 0);
    lv_style_set_bg_opa(&transparent_area_style, 0);
}
