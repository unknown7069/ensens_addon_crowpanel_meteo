#include "common.h"
lv_style_t transparent_area_style;
void ui_styles_init()
{
    lv_style_init(&transparent_area_style);
    lv_style_set_border_opa(&transparent_area_style, 0);
    lv_style_set_bg_opa(&transparent_area_style, 0);
}
number_value_t* number_value_create(lv_obj_t* parent)
{
    number_value_t* num =
        static_cast<number_value_t*>(heap_caps_malloc(sizeof(number_value_t), MALLOC_CAP_SPIRAM));
    num->cont = lv_obj_create(parent);
    lv_obj_set_width(num->cont, LV_SIZE_CONTENT);
    lv_obj_set_height(num->cont, LV_SIZE_CONTENT);
    lv_obj_clear_flag(num->cont, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_left(num->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(num->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(num->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(num->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(num->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(num->cont, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(num->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    num->int_part = lv_label_create(num->cont);
    lv_obj_set_width(num->int_part, LV_SIZE_CONTENT);
    lv_obj_set_height(num->int_part, LV_SIZE_CONTENT);
    lv_obj_set_align(num->int_part, LV_ALIGN_CENTER);
    lv_label_set_text(num->int_part, "");
    lv_obj_set_style_text_font(num->int_part, &lv_font_montserrat_26,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    num->frac_part = lv_label_create(num->cont);
    lv_obj_set_width(num->frac_part, LV_SIZE_CONTENT);
    lv_obj_set_height(num->frac_part, LV_SIZE_CONTENT);
    lv_obj_set_align(num->frac_part, LV_ALIGN_CENTER);
    lv_label_set_text(num->frac_part, "");
    lv_obj_set_style_text_font(num->frac_part, &lv_font_montserrat_12,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    num->unit_part = lv_label_create(num->cont);
    lv_obj_set_width(num->unit_part, LV_SIZE_CONTENT);
    lv_obj_set_height(num->unit_part, LV_SIZE_CONTENT);
    lv_obj_set_align(num->unit_part, LV_ALIGN_CENTER);
    lv_label_set_text(num->unit_part, "");
    lv_obj_set_style_text_font(num->unit_part, &lv_font_montserrat_12,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    static lv_coord_t grid_col_dsc[] = { LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    static lv_coord_t grid_row_dsc[] = { LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(num->cont, grid_col_dsc, grid_row_dsc);
    lv_obj_set_grid_cell(num->int_part, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 2);
    lv_obj_set_grid_cell(num->unit_part, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_grid_cell(num->frac_part, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_START, 1, 1);
    return num;
}
meter_t* tendency_meter_create(lv_obj_t* parent)
{
#define TEND_METER_WIDTH ((lv_coord_t)28)
#define TEND_METER_HEIGHT ((lv_coord_t)28)
#define MY_NEEDLE_WIDTH ((lv_coord_t)16)
#define MY_NEEDLE_HEIGHT ((lv_coord_t)5)
    meter_t* tend = static_cast<meter_t*>(heap_caps_malloc(sizeof(meter_t), MALLOC_CAP_SPIRAM));
    tend->cont = lv_obj_create(parent);
    lv_obj_set_style_pad_left(tend->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(tend->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(tend->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(tend->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(tend->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(tend->cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(tend->cont, TEND_METER_WIDTH, TEND_METER_HEIGHT);
    tend->meter = lv_meter_create(tend->cont);
    lv_obj_set_size(tend->meter, TEND_METER_WIDTH, TEND_METER_HEIGHT);
    lv_obj_remove_style(tend->meter, NULL, LV_PART_INDICATOR);
    lv_obj_set_style_border_opa(tend->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(tend->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    tend->scale = lv_meter_add_scale(tend->meter);
    lv_meter_set_scale_range(tend->meter, tend->scale, 180, 0, 180, 270);
    lv_meter_set_scale_ticks(tend->meter, tend->scale, 5, 1, 0, lv_color_black());
    lv_obj_t*           my_needle = draw_triangle(parent, MY_NEEDLE_WIDTH, MY_NEEDLE_HEIGHT);
    const lv_img_dsc_t* img_dsc   = lv_canvas_get_img(my_needle);
    lv_img_dsc_t* my_needle_img =
        lv_img_buf_alloc(img_dsc->header.w, img_dsc->header.h, img_dsc->header.cf);
    memcpy((void*)my_needle_img->data, (void*)img_dsc->data, my_needle_img->data_size);
    lv_obj_del(my_needle);
    tend->needle =
        lv_meter_add_needle_img(tend->meter, tend->scale, my_needle_img, 0, MY_NEEDLE_HEIGHT / 2);
    return tend;
}
default_box_t* default_box_create(lv_obj_t* parent)
{
    default_box_t* box =
        static_cast<default_box_t*>(heap_caps_malloc(sizeof(default_box_t), MALLOC_CAP_SPIRAM));
    box->cont = lv_obj_create(parent);
    lv_coord_t cont_size   = 130;
    lv_coord_t meter_size  = 0.93 * cont_size;
    lv_coord_t circle_size = 0.6 * meter_size;
    lv_obj_set_size(box->cont, cont_size, cont_size);
    lv_obj_clear_flag(box->cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_left(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t* meter_container = lv_obj_create(box->cont);
    lv_obj_set_size(meter_container, lv_pct(100), lv_pct(100));
    lv_obj_clear_flag(meter_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_left(meter_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(meter_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(meter_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(meter_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(meter_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    box->gauge        = (meter_t*)malloc(sizeof(meter_t));
    box->gauge->meter = lv_meter_create(meter_container);
    lv_obj_align(box->gauge->meter, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_size(box->gauge->meter, meter_size, meter_size);
    lv_obj_set_style_bg_opa(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(box->gauge->meter, NULL, LV_PART_INDICATOR);
    box->gauge->scale = lv_meter_add_scale(box->gauge->meter);
    lv_meter_set_scale_range(box->gauge->meter, box->gauge->scale, 0, 100, 270, 90);
    lv_meter_set_scale_ticks(box->gauge->meter, box->gauge->scale, 2, 1, 0,
                             lv_palette_main(LV_PALETTE_GREY));
    lv_color_t needle_color = lv_obj_get_style_text_color(parent, LV_PART_MAIN);
    box->gauge->needle =
        lv_meter_add_needle_line(box->gauge->meter, box->gauge->scale, 2, needle_color, -12);
    lv_obj_t* circle = lv_obj_create(meter_container);
    lv_obj_set_size(circle, circle_size, circle_size);
    lv_obj_align(circle, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(circle, 90, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_SCROLLABLE);
    box->tend = tendency_meter_create(circle);
    lv_obj_align(box->tend->cont, LV_ALIGN_BOTTOM_RIGHT, -4, 4);
    box->value = number_value_create(meter_container);
    lv_obj_align(box->value->cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(box->value->int_part, &lv_font_montserrat_22,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(box->value->frac_part, &lv_font_montserrat_8,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(box->cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    box->title = lv_label_create(box->cont);
    lv_obj_set_style_text_font(box->title, &lv_font_montserrat_10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(box->title, LV_ALIGN_TOP_RIGHT, 0, 0);
    box->dimension = lv_label_create(box->cont);
    lv_obj_set_style_text_font(box->dimension, &lv_font_montserrat_10,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(box->dimension, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
    return box;
}
static lv_obj_t* tabview_create_section(lv_obj_t* section_grid, lv_coord_t col, lv_coord_t row)
{
    lv_obj_t* section = lv_obj_create(section_grid);
    lv_obj_clear_flag(section, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(section, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(section, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(section, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(section, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(section, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(section, LV_GRID_ALIGN_STRETCH, col, 1, LV_GRID_ALIGN_STRETCH, row, 1);
    return section;
}

static void tabview_navigate_history_cb(lv_event_t* e)
{
    tabview_t* tview = static_cast<tabview_t*>(lv_event_get_user_data(e));
    if (tview == nullptr || tview->cont == nullptr || tview->tab_history == nullptr)
        return;

    uint32_t history_index = lv_obj_get_index(tview->tab_history);
    lv_tabview_set_act(tview->cont, history_index, LV_ANIM_ON);
}

static void tabview_make_metric_clickable(lv_obj_t* obj, tabview_t* tview)
{
    if (obj == nullptr || tview == nullptr)
        return;

    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, tabview_navigate_history_cb, LV_EVENT_CLICKED, tview);
}

namespace
{
constexpr uint16_t kBottomPlotSlotCount = 48;
} // namespace

static void bottom_plot_x_axis_label_event_cb(lv_event_t* e)
{
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
    if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL))
        return;
    if (dsc->id != LV_CHART_AXIS_PRIMARY_X || dsc->text == nullptr)
        return;

    int32_t hours = static_cast<int32_t>(dsc->value) * 4;
    lv_snprintf(dsc->text, dsc->text_length, "%dh", static_cast<int>(hours));
}

static void bottom_plot_y_axis_label_event_cb(lv_event_t* e)
{
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
    if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL))
        return;
    if (dsc->id != LV_CHART_AXIS_PRIMARY_Y || dsc->text == nullptr)
        return;

    lv_snprintf(dsc->text, dsc->text_length, "%d", (int)dsc->value);
}

static void bottom_plot_cursor_draw_event_cb(lv_event_t* e)
{
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
    if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_CURSOR))
        return;

    if (dsc->line_dsc != nullptr)
    {
        dsc->line_dsc->color      = lv_color_white();
        dsc->line_dsc->width      = 2;
        dsc->line_dsc->dash_width = 4;
        dsc->line_dsc->dash_gap   = 4;
    }
}

static void tabview_init_time_section(tabview_t* tview, lv_obj_t* section_grid)
{
    lv_obj_t* time_section = tabview_create_section(section_grid, 0, 0);
    lv_obj_set_layout(time_section, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(time_section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(time_section, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    tview->label = lv_label_create(time_section);
    lv_label_set_text(tview->label, "--:--");
    lv_obj_set_style_text_font(tview->label, &saira_condensed_medium, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    tabview_make_metric_clickable(tview->label, tview);
    tview->date_label = lv_label_create(time_section);
    lv_label_set_text(tview->date_label, "--");
    lv_obj_set_style_text_font(tview->date_label, &lv_font_montserrat_32, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->date_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    tabview_make_metric_clickable(tview->date_label, tview);
}
static void tabview_init_outdoor_section(tabview_t* tview, lv_obj_t* section_grid)
{
    lv_obj_t* outdoor_section = tabview_create_section(section_grid, 0, 1);
    lv_obj_set_grid_cell(outdoor_section, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_layout(outdoor_section, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_row(outdoor_section, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(outdoor_section, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    static lv_coord_t outdoor_col_dsc[] = { LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(1),
                                            LV_GRID_TEMPLATE_LAST };
    static lv_coord_t outdoor_row_dsc[] = { LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT,
                                            LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(outdoor_section, outdoor_col_dsc, outdoor_row_dsc);

    lv_obj_t* temperature_icon_obj = lv_img_create(outdoor_section);
    lv_img_set_src(temperature_icon_obj, &temperature);
    lv_obj_set_grid_cell(temperature_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    tview->temp_outside_label = lv_label_create(outdoor_section);
    lv_label_set_text(tview->temp_outside_label, "--");
    lv_obj_set_style_text_font(tview->temp_outside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->temp_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->temp_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    tabview_make_metric_clickable(tview->temp_outside_label, tview);

    lv_obj_t* feels_like_icon = lv_label_create(outdoor_section);
    lv_label_set_text(feels_like_icon, "FL");
    lv_obj_set_style_text_font(feels_like_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(feels_like_icon, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    tview->feels_like_label = lv_label_create(outdoor_section);
    lv_label_set_text(tview->feels_like_label, "--");
    lv_obj_set_style_text_font(tview->feels_like_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->feels_like_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->feels_like_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    tabview_make_metric_clickable(tview->feels_like_label, tview);

    lv_obj_t* humidity_icon_obj = lv_img_create(outdoor_section);
    lv_img_set_src(humidity_icon_obj, &humidity_icon);
    lv_obj_set_grid_cell(humidity_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    tview->humidity_outside_label = lv_label_create(outdoor_section);
    lv_label_set_text(tview->humidity_outside_label, "--%");
    lv_obj_set_style_text_font(tview->humidity_outside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->humidity_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->humidity_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tabview_make_metric_clickable(tview->humidity_outside_label, tview);

    lv_obj_t* pressure_icon_obj = lv_img_create(outdoor_section);
    lv_img_set_src(pressure_icon_obj, &pressure_icon);
    lv_obj_set_grid_cell(pressure_icon_obj, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    tview->pressure_outside_label = lv_label_create(outdoor_section);
    lv_label_set_text(tview->pressure_outside_label, "--");
    lv_obj_set_style_text_font(tview->pressure_outside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->pressure_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->pressure_outside_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tabview_make_metric_clickable(tview->pressure_outside_label, tview);

}

static void tabview_init_outdoor_details_section(tabview_t* tview, lv_obj_t* section_grid)
{
    lv_obj_t* outdoor_details_section = tabview_create_section(section_grid, 1, 1);
    lv_obj_set_grid_cell(outdoor_details_section, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_layout(outdoor_details_section, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_row(outdoor_details_section, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(outdoor_details_section, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    static lv_coord_t outdoor_details_col_dsc[] = { LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t outdoor_details_row_dsc[] = { LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT,
                                                   LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(outdoor_details_section, outdoor_details_col_dsc, outdoor_details_row_dsc);

    lv_obj_t* wind_icon_obj = lv_img_create(outdoor_details_section);
    lv_img_set_src(wind_icon_obj, &wind_icon);
    lv_obj_set_grid_cell(wind_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    tview->wind_speed_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(tview->wind_speed_label, "--");
    lv_obj_set_style_text_font(tview->wind_speed_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->wind_speed_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->wind_speed_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    tabview_make_metric_clickable(tview->wind_speed_label, tview);

    lv_obj_t* high_icon = lv_label_create(outdoor_details_section);
    lv_label_set_text(high_icon, LV_SYMBOL_UP);
    lv_obj_set_style_text_font(high_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(high_icon, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    tview->daily_high_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(tview->daily_high_label, "--");
    lv_obj_set_style_text_font(tview->daily_high_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->daily_high_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->daily_high_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tabview_make_metric_clickable(tview->daily_high_label, tview);

    lv_obj_t* low_icon = lv_label_create(outdoor_details_section);
    lv_label_set_text(low_icon, LV_SYMBOL_DOWN);
    lv_obj_set_style_text_font(low_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(low_icon, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    tview->daily_low_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(tview->daily_low_label, "--");
    lv_obj_set_style_text_font(tview->daily_low_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->daily_low_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->daily_low_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tabview_make_metric_clickable(tview->daily_low_label, tview);

    lv_obj_t* precipitation_icon = lv_label_create(outdoor_details_section);
    lv_label_set_text(precipitation_icon, "Rain 24h");
    lv_obj_set_style_text_font(precipitation_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(precipitation_icon, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    tview->precipitation_outside_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(tview->precipitation_outside_label, "-- mm");
    lv_obj_set_style_text_font(tview->precipitation_outside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->precipitation_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->precipitation_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    tabview_make_metric_clickable(tview->precipitation_outside_label, tview);
}

static void tabview_init_history_tab(tabview_t* tview)
{
    if (tview == nullptr)
        return;

    tview->tab_history = lv_tabview_add_tab(tview->cont, "History");
    lv_obj_set_style_pad_top(tview->tab_history, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(tview->tab_history, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* history_root = lv_obj_create(tview->tab_history);
    lv_obj_set_size(history_root, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(history_root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(history_root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(history_root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(history_root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(history_root, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(history_root, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_layout(history_root, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(history_root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(history_root, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    tview->bottom_plot_title = lv_label_create(history_root);
    lv_label_set_text(tview->bottom_plot_title, "History");
    lv_obj_set_width(tview->bottom_plot_title, LV_PCT(100));
    lv_obj_set_style_text_font(tview->bottom_plot_title, &lv_font_montserrat_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->bottom_plot_title, LV_TEXT_ALIGN_CENTER,
                                LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* plot_container = lv_obj_create(history_root);
    lv_obj_set_width(plot_container, LV_PCT(100));
    lv_obj_set_flex_grow(plot_container, 1);
    lv_obj_clear_flag(plot_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(plot_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(plot_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(plot_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_layout(plot_container, LV_LAYOUT_GRID);
    static lv_coord_t plot_grid_cols[] = { LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t plot_grid_rows[] = { LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(plot_container, plot_grid_cols, plot_grid_rows);

    lv_obj_t* plot_left_spacer = lv_obj_create(plot_container);
    lv_obj_set_size(plot_left_spacer, 28, LV_PCT(100));
    lv_obj_clear_flag(plot_left_spacer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(plot_left_spacer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(plot_left_spacer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(plot_left_spacer, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    tview->bottom_plot_chart = lv_chart_create(plot_container);
    lv_obj_set_size(tview->bottom_plot_chart, LV_PCT(100), LV_PCT(100));
    lv_obj_set_grid_cell(tview->bottom_plot_chart, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH,
                         0, 1);
    lv_obj_set_style_pad_all(tview->bottom_plot_chart, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(tview->bottom_plot_chart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(tview->bottom_plot_chart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_chart_set_type(tview->bottom_plot_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(tview->bottom_plot_chart, kBottomPlotSlotCount);
    lv_chart_set_range(tview->bottom_plot_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_axis_tick(tview->bottom_plot_chart, LV_CHART_AXIS_PRIMARY_Y, 6, 3, 6, 2, true, 40);
    lv_chart_set_axis_tick(tview->bottom_plot_chart, LV_CHART_AXIS_PRIMARY_X, 6, 3, 7, 4, true, 30);
    lv_obj_add_event_cb(tview->bottom_plot_chart, bottom_plot_x_axis_label_event_cb,
                        LV_EVENT_DRAW_PART_BEGIN, nullptr);
    lv_obj_add_event_cb(tview->bottom_plot_chart, bottom_plot_y_axis_label_event_cb,
                        LV_EVENT_DRAW_PART_BEGIN, nullptr);
    lv_obj_add_event_cb(tview->bottom_plot_chart, bottom_plot_cursor_draw_event_cb,
                        LV_EVENT_DRAW_PART_BEGIN, nullptr);
    lv_obj_set_style_text_font(tview->bottom_plot_chart, &lv_font_montserrat_12,
                               LV_PART_TICKS | LV_STATE_DEFAULT);
    tview->bottom_plot_series = lv_chart_add_series(tview->bottom_plot_chart,
                                                    lv_palette_main(LV_PALETTE_LIGHT_BLUE),
                                                    LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_all_value(tview->bottom_plot_chart, tview->bottom_plot_series, 0);
    tview->bottom_plot_cursor =
        lv_chart_add_cursor(tview->bottom_plot_chart, lv_color_white(), LV_DIR_VER);
    if (tview->bottom_plot_cursor)
    {
        lv_point_t origin = { 0, 0 };
        lv_chart_set_cursor_pos(tview->bottom_plot_chart, tview->bottom_plot_cursor, &origin);
    }

    lv_obj_t* plot_bottom_spacer = lv_obj_create(plot_container);
    lv_obj_set_size(plot_bottom_spacer, LV_PCT(100), 24);
    lv_obj_clear_flag(plot_bottom_spacer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(plot_bottom_spacer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(plot_bottom_spacer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(plot_bottom_spacer, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
}
static void tabview_init_indoor_section(tabview_t* tview, lv_obj_t* section_grid)
{
    lv_obj_t* indoor_section = tabview_create_section(section_grid, 1, 0);
    lv_obj_set_grid_cell(indoor_section, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_layout(indoor_section, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_row(indoor_section, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(indoor_section, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    static lv_coord_t indoor_col_dsc[] = { LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t indoor_row_dsc[] = { LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(indoor_section, indoor_col_dsc, indoor_row_dsc);
    lv_obj_t* temperature_icon_indoor = lv_img_create(indoor_section);
    lv_img_set_src(temperature_icon_indoor, &temperature);
    lv_obj_set_grid_cell(temperature_icon_indoor, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    tview->temp_inside_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->temp_inside_label, "--");
    lv_obj_set_style_text_font(tview->temp_inside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->temp_inside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->temp_inside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    tabview_make_metric_clickable(tview->temp_inside_label, tview);
    lv_obj_t* humidity_icon_indoor = lv_img_create(indoor_section);
    lv_img_set_src(humidity_icon_indoor, &humidity_icon);
    lv_obj_set_grid_cell(humidity_icon_indoor, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    tview->humidity_inside_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->humidity_inside_label, "--%");
    lv_obj_set_style_text_font(tview->humidity_inside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->humidity_inside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->humidity_inside_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    tabview_make_metric_clickable(tview->humidity_inside_label, tview);
    lv_obj_t* pressure_icon_indoor = lv_img_create(indoor_section);
    lv_img_set_src(pressure_icon_indoor, &pressure_icon);
    lv_obj_set_grid_cell(pressure_icon_indoor, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tview->pressure_inside_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->pressure_inside_label, "--");
    lv_obj_set_style_text_font(tview->pressure_inside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->pressure_inside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->pressure_inside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tabview_make_metric_clickable(tview->pressure_inside_label, tview);
    lv_obj_t* voc_title_label = lv_label_create(indoor_section);
    lv_label_set_text(voc_title_label, "VOC");
    lv_obj_set_style_text_font(voc_title_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(voc_title_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(voc_title_label, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tview->voc_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->voc_label, "--");
    lv_obj_set_style_text_font(tview->voc_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->voc_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->voc_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tabview_make_metric_clickable(tview->voc_label, tview);
    lv_obj_t* co2_icon_obj = lv_img_create(indoor_section);
    lv_img_set_src(co2_icon_obj, &co2);
    lv_obj_set_grid_cell(co2_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tview->co2_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->co2_label, "--");
    lv_obj_set_style_text_font(tview->co2_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->co2_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->co2_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tabview_make_metric_clickable(tview->co2_label, tview);
    lv_obj_t* iaq_icon_obj = lv_img_create(indoor_section);
    lv_img_set_src(iaq_icon_obj, &aqi);
    lv_obj_set_grid_cell(iaq_icon_obj, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tview->iaq_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->iaq_label, "--");
    lv_obj_set_style_text_font(tview->iaq_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->iaq_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->iaq_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tabview_make_metric_clickable(tview->iaq_label, tview);
}
static void tabview_init_dashboard_tab(tabview_t* tview)
{
    if (tview == nullptr)
        return;
    tview->tab_dashboard = lv_tabview_add_tab(tview->cont, "Dashboard");
    lv_obj_set_style_pad_top(tview->tab_dashboard, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(tview->tab_dashboard, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t* section_grid = lv_obj_create(tview->tab_dashboard);
    lv_obj_set_size(section_grid, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(section_grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_layout(section_grid, LV_LAYOUT_GRID);
    lv_obj_set_style_bg_opa(section_grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(section_grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(section_grid, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(section_grid, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(section_grid, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    static lv_coord_t section_col_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t section_row_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(section_grid, section_col_dsc, section_row_dsc);
    tabview_init_time_section(tview, section_grid);
    tabview_init_outdoor_section(tview, section_grid);
    tabview_init_outdoor_details_section(tview, section_grid);
    tabview_init_indoor_section(tview, section_grid);

    lv_obj_t* horizontal_divider = lv_obj_create(section_grid);
    lv_obj_remove_style_all(horizontal_divider);
    lv_obj_set_size(horizontal_divider, LV_PCT(100), 2);
    lv_obj_set_style_bg_color(horizontal_divider, lv_palette_main(LV_PALETTE_GREY),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(horizontal_divider, LV_OPA_40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(horizontal_divider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(horizontal_divider, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_align(horizontal_divider, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* vertical_divider = lv_obj_create(section_grid);
    lv_obj_remove_style_all(vertical_divider);
    lv_obj_set_size(vertical_divider, 2, LV_PCT(100));
    lv_obj_set_style_bg_color(vertical_divider, lv_palette_main(LV_PALETTE_GREY),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(vertical_divider, LV_OPA_40, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(vertical_divider, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_flag(vertical_divider, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_align(vertical_divider, LV_ALIGN_CENTER, 0, 0);
}
tabview_t* tabview_create(lv_obj_t* parent, int32_t tab_h)
{
    tabview_t* tview =
        static_cast<tabview_t*>(heap_caps_malloc(sizeof(tabview_t), MALLOC_CAP_SPIRAM));
    tview->tab_h  = tab_h;
    tview->parent = parent;
    tview->cont   = lv_tabview_create(parent, LV_DIR_TOP, tview->tab_h);
    tabview_init_dashboard_tab(tview);
    tabview_init_history_tab(tview);
    tview->tab_settings = lv_tabview_add_tab(tview->cont, "Settings");
    lv_obj_set_style_pad_top(tview->tab_settings, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(tview->tab_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    tview->tab_1 = lv_tabview_add_tab(tview->cont, "Indoor");
    /* Remove tab button area padding */
    lv_obj_set_style_pad_left(tview->tab_1, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(tview->tab_1, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(tview->tab_1, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(tview->tab_1, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    tview->tab_2 = lv_tabview_add_tab(tview->cont, "Forecast");
    /* Remove tab button area padding */
    // lv_obj_set_style_pad_left(tview->tab_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_pad_right(tview->tab_2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(tview->tab_2, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(tview->tab_2, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    tview->tab_bar = lv_tabview_get_tab_btns(tview->cont);
    lv_obj_set_style_pad_right(tview->tab_bar, LV_HOR_RES / 2, 0);
    return tview;
}
data_box_t* data_box_create(lv_obj_t* parent)
{
    data_box_t* box =
        static_cast<data_box_t*>(heap_caps_malloc(sizeof(data_box_t), MALLOC_CAP_SPIRAM));
    box->cont = lv_obj_create(parent);
    lv_obj_clear_flag(box->cont, LV_OBJ_FLAG_SCROLLABLE);
    box->value = number_value_create(box->cont);
    box->tend  = tendency_meter_create(box->cont);
    lv_meter_set_indicator_value(box->tend->meter, box->tend->needle, 90);
    static lv_coord_t grid_col_dsc[] = { 50, 5, LV_GRID_TEMPLATE_LAST };
    static lv_coord_t grid_row_dsc[] = { LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(box->cont, grid_col_dsc, grid_row_dsc);
    lv_obj_set_grid_cell(box->value->cont, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_END, 0, 1);
    lv_obj_set_grid_cell(box->tend->cont, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 0, 1);
    return box;
}
humi_dew_data_box_t* humi_dew_box_create(lv_obj_t* parent)
{
    humi_dew_data_box_t* box = static_cast<humi_dew_data_box_t*>(
        heap_caps_malloc(sizeof(humi_dew_data_box_t), MALLOC_CAP_SPIRAM));
    box->cont = lv_obj_create(parent);
    lv_obj_clear_flag(box->cont, LV_OBJ_FLAG_SCROLLABLE);
    box->value_humi = number_value_create(box->cont);
    box->tend       = tendency_meter_create(box->cont);
    lv_meter_set_indicator_value(box->tend->meter, box->tend->needle, 90);
    box->value_dew = number_value_create(box->cont);
    box->title_dew = lv_label_create(box->cont);
    box->humiIcon = lv_img_create(box->cont);
    lv_img_set_src(box->humiIcon, &humidity_icon);
    lv_obj_set_style_text_font(box->title_dew, &lv_font_montserrat_10,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    static lv_coord_t grid_col_dsc[] = { 5, 53, 5, 75, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    static lv_coord_t grid_row_dsc[] = { LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(box->cont, grid_col_dsc, grid_row_dsc);
    lv_obj_set_grid_cell(box->humiIcon, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(box->value_humi->cont, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_END, 0,
                         1);
    lv_obj_set_grid_cell(box->tend->cont, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_grid_cell(box->value_dew->cont, LV_GRID_ALIGN_END, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(box->title_dew, LV_GRID_ALIGN_START, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    return box;
}
days_header_t* days_header_create(lv_obj_t* parent)
{
    days_header_t* box =
        static_cast<days_header_t*>(heap_caps_malloc(sizeof(days_header_t), MALLOC_CAP_SPIRAM));
    static const char* day_names_[] = { "Su", "Mo", "Tu", "We", "Th", "Fr", "Sa" };
    box->cont                       = lv_obj_create(parent);
    lv_obj_clear_flag(box->cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_left(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(box->cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(box->cont, 20, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    for (int i = 0; i < 7; i++)
    {
        box->days_labels[i] = lv_label_create(box->cont);
        lv_label_set_text(box->days_labels[i], day_names_[i]);
        lv_obj_set_style_text_font(box->days_labels[i], &lv_font_montserrat_16,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        box->current_day[i] = lv_label_create(box->cont);
        lv_label_set_text(box->current_day[i], CUR_DAY_SYMBOL);
        lv_obj_set_style_text_font(box->current_day[i], &lv_font_montserrat_48,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    box->zodizk_sign = lv_label_create(box->cont);
    lv_label_set_text(box->zodizk_sign, "Loading:...");
    lv_obj_set_style_text_font(box->zodizk_sign, &lv_font_montserrat_20,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    static lv_coord_t days_grid_col_dsc[] = { LV_GRID_CONTENT, LV_GRID_CONTENT,
                                              LV_GRID_CONTENT, LV_GRID_CONTENT,
                                              LV_GRID_CONTENT, LV_GRID_CONTENT,
                                              LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    static lv_coord_t days_grid_row_dsc[] = { 5, LV_GRID_CONTENT, LV_GRID_CONTENT,
                                              LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(box->cont, days_grid_col_dsc, days_grid_row_dsc);
    for (int i = 0; i < 7; i++)
    {
        lv_obj_set_grid_cell(box->days_labels[i], LV_GRID_ALIGN_CENTER, i, 1, LV_GRID_ALIGN_CENTER,
                             1, 1);
        lv_obj_set_grid_cell(box->current_day[i], LV_GRID_ALIGN_CENTER, i, 1, LV_GRID_ALIGN_CENTER,
                             0, 1);
    }
    lv_obj_set_grid_cell(box->zodizk_sign, LV_GRID_ALIGN_START, 0, 7, LV_GRID_ALIGN_END, 2, 1);
    return box;
}
lv_obj_t* draw_triangle(lv_obj_t* parent, lv_coord_t width, lv_coord_t height)
{
    lv_obj_t* canvas = lv_canvas_create(parent);
    lv_obj_set_size(canvas, width, height);
    lv_color_t* cbuf = static_cast<lv_color_t*>(
        heap_caps_malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(width, height), MALLOC_CAP_SPIRAM));
    lv_canvas_set_buffer(canvas, cbuf, width, height, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_TRANSP);
    lv_point_t arrow_points[3];
    arrow_points[0].x = 0;
    arrow_points[0].y = 0;
    arrow_points[1].x = 0;
    arrow_points[1].y = height;
    arrow_points[2].x = width;
    arrow_points[2].y = height / 2;
    lv_color_t color = lv_obj_get_style_text_color(parent, LV_PART_MAIN);
    lv_draw_rect_dsc_t poly_dsc;
    lv_draw_rect_dsc_init(&poly_dsc);
    poly_dsc.bg_color     = color;
    poly_dsc.border_width = 0;
    lv_canvas_draw_polygon(canvas, arrow_points, 3, &poly_dsc);
    return canvas;
}
lv_obj_t* draw_filled_half_circle(lv_obj_t* parent, lv_coord_t radius, lv_color_t color)
{
    lv_coord_t width  = radius * 2;
    lv_coord_t height = radius;
    lv_obj_t* canvas = lv_canvas_create(parent);
    lv_obj_set_size(canvas, width, height);
    lv_color_t* cbuf = static_cast<lv_color_t*>(
        heap_caps_malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR_ALPHA(width, height), MALLOC_CAP_SPIRAM));
    lv_canvas_set_buffer(canvas, cbuf, width, height, LV_IMG_CF_TRUE_COLOR_ALPHA);
    lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_TRANSP);
    lv_draw_arc_dsc_t arc_dsc;
    lv_draw_arc_dsc_init(&arc_dsc);
    arc_dsc.color = color;
    arc_dsc.width = radius;
    lv_canvas_draw_arc(canvas, radius, radius, radius, 180, 360, &arc_dsc);
    return canvas;
}
lv_obj_t* draw_circle(lv_obj_t* parent, lv_coord_t radius)
{
    lv_coord_t width  = radius * 2;
    lv_coord_t height = radius * 2;
    lv_obj_t*  circle = lv_obj_create(parent);
    lv_obj_set_size(circle, width, height);
    lv_obj_set_style_radius(circle, 90, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(circle, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(circle, LV_OBJ_FLAG_SCROLLABLE);
    return circle;
}


