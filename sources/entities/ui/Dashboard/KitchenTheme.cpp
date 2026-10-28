#include "KitchenTheme.h"

namespace KitchenTheme {
namespace {
lv_theme_t theme;
lv_style_t paper, ink, control, pressed, accent, selected, muted, key, disabled;
bool initialized = false;

void apply(lv_theme_t*, lv_obj_t* obj)
{
    if (!lv_obj_check_type(obj, &lv_label_class))
        lv_obj_add_style(obj, &ink, 0);
    lv_obj_set_style_bg_color(obj, lv_color_hex(Rule), LV_PART_SCROLLBAR);
    if (lv_obj_check_type(obj, &lv_obj_class) || lv_obj_check_type(obj, &lv_menu_class) ||
        lv_obj_check_type(obj, &lv_menu_page_class) || lv_obj_check_type(obj, &lv_menu_main_cont_class) ||
        lv_obj_check_type(obj, &lv_menu_main_header_cont_class))
        lv_obj_add_style(obj, &paper, 0);
    if (lv_obj_check_type(obj, &lv_btn_class)) {
        lv_obj_add_style(obj, &control, 0);
        lv_obj_add_style(obj, &pressed, LV_STATE_PRESSED);
        lv_obj_set_style_min_height(obj, 44, 0);
        lv_obj_set_style_shadow_width(obj, 0, LV_STATE_PRESSED);
    }
    if (lv_obj_check_type(obj, &lv_dropdown_class) || lv_obj_check_type(obj, &lv_textarea_class) ||
        lv_obj_check_type(obj, &lv_dropdownlist_class) || lv_obj_check_type(obj, &lv_calendar_class)) {
        lv_obj_add_style(obj, &control, 0);
        lv_obj_set_style_border_color(obj, lv_color_hex(Terracotta), LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &disabled, LV_STATE_DISABLED);
    }
    if (lv_obj_check_type(obj, &lv_dropdownlist_class)) {
        lv_obj_add_style(obj, &selected, LV_PART_SELECTED);
        lv_obj_add_style(obj, &pressed, LV_PART_SELECTED | LV_STATE_PRESSED);
    }
    if (lv_obj_check_type(obj, &lv_menu_sidebar_cont_class) ||
        lv_obj_check_type(obj, &lv_menu_sidebar_header_cont_class)) {
        lv_obj_add_style(obj, &control, 0);
        lv_obj_set_style_border_width(obj, 0, 0);
        lv_obj_set_style_radius(obj, 0, 0);
    }
    if (lv_obj_check_type(obj, &lv_menu_cont_class)) {
        lv_obj_add_style(obj, &selected, LV_STATE_CHECKED);
        lv_obj_add_style(obj, &pressed, LV_STATE_PRESSED);
        lv_obj_set_style_radius(obj, 8, 0);
    }
    if (lv_obj_check_type(obj, &lv_switch_class) || lv_obj_check_type(obj, &lv_slider_class)) {
        lv_obj_set_style_bg_color(obj, lv_color_hex(Rule), LV_PART_MAIN);
        lv_obj_add_style(obj, &accent, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &accent, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &disabled, LV_STATE_DISABLED);
        lv_obj_set_style_bg_color(obj, lv_color_hex(Paper), LV_PART_KNOB);
        lv_obj_set_style_shadow_width(obj, 0, LV_PART_KNOB);
    }
    if (lv_obj_check_type(obj, &lv_checkbox_class)) {
        lv_obj_add_style(obj, &control, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &accent, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_border_color(obj, lv_color_hex(Terracotta), LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(obj, lv_color_hex(Paper), LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_pad_column(obj, 12, 0);
    }
    if (lv_obj_check_type(obj, &lv_keyboard_class) || lv_obj_check_type(obj, &lv_btnmatrix_class)) {
        lv_obj_add_style(obj, &paper, 0);
        lv_obj_add_style(obj, &key, LV_PART_ITEMS);
        lv_obj_add_style(obj, &pressed, LV_PART_ITEMS | LV_STATE_PRESSED);
        lv_obj_add_style(obj, &selected, LV_PART_ITEMS | LV_STATE_CHECKED);
        lv_obj_set_style_text_color(obj, lv_color_hex(Muted), LV_PART_ITEMS | LV_STATE_DISABLED);
        lv_obj_set_style_text_color(obj, lv_color_hex(Terracotta), LV_PART_ITEMS | LV_STATE_USER_1);
    }
    if (lv_obj_check_type(obj, &lv_chart_class)) {
        lv_obj_set_style_line_color(obj, lv_color_hex(Rule), LV_PART_MAIN);
        lv_obj_set_style_line_opa(obj, LV_OPA_40, LV_PART_MAIN);
        lv_obj_set_style_line_width(obj, 2, LV_PART_ITEMS);
    }
}
}

void initialize(lv_disp_t* display)
{
    auto* base = lv_theme_default_init(display, lv_color_hex(Terracotta),
                                       lv_color_hex(Slate), false, &lv_font_montserrat_22);
    if (!initialized) {
        lv_style_init(&paper);
        lv_style_set_bg_color(&paper, lv_color_hex(Paper));
        lv_style_set_text_color(&paper, lv_color_hex(Ink));
        lv_style_init(&ink);
        lv_style_set_text_color(&ink, lv_color_hex(Ink));
        lv_style_init(&control);
        lv_style_set_bg_color(&control, lv_color_hex(Oatmeal));
        lv_style_set_bg_opa(&control, LV_OPA_COVER);
        lv_style_set_text_color(&control, lv_color_hex(Ink));
        lv_style_set_text_font(&control, &lv_font_montserrat_22);
        lv_style_set_border_color(&control, lv_color_hex(Rule));
        lv_style_set_border_width(&control, 1);
        lv_style_set_radius(&control, 8);
        lv_style_set_shadow_width(&control, 0);
        lv_style_set_pad_all(&control, 12);
        lv_style_init(&pressed);
        lv_style_set_bg_color(&pressed, lv_color_hex(Rule));
        lv_style_set_text_color(&pressed, lv_color_hex(Ink));
        lv_style_set_shadow_width(&pressed, 0);
        lv_style_init(&accent);
        lv_style_set_bg_color(&accent, lv_color_hex(Terracotta));
        lv_style_set_bg_opa(&accent, LV_OPA_COVER);
        lv_style_init(&selected);
        lv_style_set_bg_color(&selected, lv_color_hex(Terracotta));
        lv_style_set_bg_opa(&selected, LV_OPA_COVER);
        lv_style_set_text_color(&selected, lv_color_hex(Paper));
        lv_style_init(&muted);
        lv_style_set_text_color(&muted, lv_color_hex(Muted));
        lv_style_init(&key);
        lv_style_set_bg_color(&key, lv_color_hex(Oatmeal));
        lv_style_set_bg_opa(&key, LV_OPA_COVER);
        lv_style_set_text_color(&key, lv_color_hex(Ink));
        lv_style_set_text_font(&key, &lv_font_montserrat_22);
        lv_style_set_border_width(&key, 0);
        lv_style_set_shadow_width(&key, 0);
        lv_style_set_radius(&key, 6);
        lv_style_init(&disabled);
        lv_style_set_opa(&disabled, LV_OPA_40);
        initialized = true;
    }
    theme = *base;
    lv_theme_set_parent(&theme, base);
    lv_theme_set_apply_cb(&theme, apply);
    lv_disp_set_theme(display, &theme);
    lv_obj_set_style_bg_color(lv_disp_get_scr_act(display), lv_color_hex(Paper), 0);
}

void surface(lv_obj_t* object, uint32_t color, int radius)
{
    lv_obj_set_style_bg_color(object, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(object, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(object, lv_color_hex(Ink), 0);
    lv_obj_set_style_border_width(object, 0, 0);
    lv_obj_set_style_shadow_width(object, 0, 0);
    lv_obj_set_style_radius(object, radius, 0);
    lv_obj_set_style_pad_all(object, 0, 0);
    lv_obj_set_style_pad_gap(object, 0, 0);
}
}
