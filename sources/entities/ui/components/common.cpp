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

tabview_t* tabview_create(lv_obj_t* parent, int32_t tab_h)
{
    tabview_t* tview =
        static_cast<tabview_t*>(heap_caps_malloc(sizeof(tabview_t), MALLOC_CAP_SPIRAM));
    tview->tab_h  = tab_h;
    tview->parent = parent;
    tview->cont   = lv_tabview_create(parent, LV_DIR_TOP, tview->tab_h);

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
