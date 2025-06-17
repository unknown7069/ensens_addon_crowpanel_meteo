#include "pressure_box.h"

pressure_box_t* pressure_box_create(lv_obj_t* parent)
{
    pressure_box_t* box =
        static_cast<pressure_box_t*>(heap_caps_malloc(sizeof(pressure_box_t), MALLOC_CAP_SPIRAM));

    box->cont = lv_obj_create(parent);
    lv_obj_set_size(box->cont, 310, LV_SIZE_CONTENT);
    lv_obj_clear_flag(box->cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_left(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(box->cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(box->cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_row(box->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* meter_container = lv_obj_create(box->cont);
    lv_obj_set_size(meter_container, lv_pct(100), 110);
    lv_obj_clear_flag(meter_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(meter_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(meter_container, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    box->title = lv_label_create(meter_container);
    lv_obj_set_style_text_font(box->title, &lv_font_montserrat_12, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(box->title, LV_ALIGN_TOP_RIGHT, -5, 0);
    lv_label_set_text(box->title, "hPa");

    int32_t scale_min = 900;
    int32_t scale_max = 1100;

    box->gauge        = static_cast<meter_t*>(heap_caps_malloc(sizeof(meter_t), MALLOC_CAP_SPIRAM));
    box->gauge->meter = lv_meter_create(meter_container);
    lv_obj_align(box->gauge->meter, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_size(box->gauge->meter, 220, 220);
    lv_obj_set_style_bg_opa(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(box->gauge->meter, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(box->gauge->meter, 30, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(box->gauge->meter, 16, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(box->gauge->meter, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_remove_style(box->gauge->meter, NULL, LV_PART_INDICATOR);

    const lv_color_t major_ticks_color = lv_obj_get_style_text_color(parent, LV_PART_MAIN);

    box->gauge->scale = lv_meter_add_scale(box->gauge->meter);
    lv_meter_set_scale_range(box->gauge->meter, box->gauge->scale, scale_min, scale_max, 180, 180);
    lv_meter_set_scale_ticks(box->gauge->meter, box->gauge->scale, 31, 1, 4,
                             lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(box->gauge->meter, box->gauge->scale, 5, 2, 6, major_ticks_color,
                                   -20);
    lv_obj_set_style_text_font(box->gauge->meter, &lv_font_montserrat_12,
                               LV_PART_TICKS | LV_STATE_DEFAULT);

    box->gauge->needle = lv_meter_add_needle_line(box->gauge->meter, box->gauge->scale, 2,
                                                  lv_palette_main(LV_PALETTE_RED), -10);
    lv_meter_set_indicator_value(box->gauge->meter, box->gauge->needle, 1013);

    const lv_color_t bg_color    = lv_obj_get_style_bg_color(box->cont, LV_PART_MAIN);
    lv_obj_t*        half_circle = draw_filled_half_circle(meter_container, 60, bg_color);
    lv_obj_align(half_circle, LV_ALIGN_BOTTOM_MID, 0, -10);

    box->tend = tendency_meter_create(meter_container);
    lv_obj_align(box->tend->cont, LV_ALIGN_BOTTOM_MID, 40, -5);
    lv_meter_set_indicator_value(box->tend->meter, box->tend->needle, 90);

    box->value = number_value_create(meter_container);
    lv_obj_align(box->value->cont, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_label_set_text(box->value->int_part, "-");

    box->plot.cont = lv_obj_create(box->cont);
    lv_obj_set_size(box->plot.cont, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_pad_top(box->plot.cont, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(box->plot.cont, 15, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(box->plot.cont, 35, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(box->plot.cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    box->plot.chart = lv_chart_create(box->plot.cont);
    lv_obj_set_size(box->plot.chart, lv_pct(100), 180);

    lv_obj_set_style_pad_all(box->plot.chart, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(box->plot.chart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(box->plot.chart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_size(box->plot.chart, 2, LV_PART_INDICATOR);

    lv_chart_set_type(box->plot.chart, LV_CHART_TYPE_BAR);
    lv_chart_set_range(box->plot.chart, LV_CHART_AXIS_SECONDARY_Y, scale_min * MULT_COEFF,
                       scale_max * MULT_COEFF);

    lv_chart_set_axis_tick(box->plot.chart, LV_CHART_AXIS_SECONDARY_Y, 0, 0, 7, 10, true, 40);
    lv_chart_set_axis_tick(box->plot.chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, 7, 10, true, 30);
    lv_obj_set_style_text_font(box->plot.chart, &lv_font_montserrat_12,
                               LV_PART_TICKS | LV_STATE_DEFAULT);

    static lv_style_t chartLabelsStyle;
    lv_style_init(&chartLabelsStyle);
    lv_style_set_text_color(&chartLabelsStyle, lv_color_hex(0xFFFFFF));
    lv_obj_add_style(box->plot.chart, &chartLabelsStyle, LV_PART_TICKS | LV_STATE_DEFAULT);

    box->plot.y_sec =
        lv_chart_add_series(box->plot.chart, lv_color_hex(0x2095F6), LV_CHART_AXIS_SECONDARY_Y);
    lv_chart_set_update_mode(box->plot.chart, LV_CHART_UPDATE_MODE_SHIFT);

    static lv_coord_t panel_grid_col_dsc[] = { LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    static lv_coord_t panel_grid_row_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(2),
                                               LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(box->cont, panel_grid_col_dsc, panel_grid_row_dsc);
    lv_obj_set_grid_cell(meter_container, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_grid_cell(box->plot.cont, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    return box;
}
