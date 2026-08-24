#include "common.h"
lv_style_t transparent_area_style;
void ui_styles_init()
{
    lv_style_init(&transparent_area_style);
    lv_style_set_border_opa(&transparent_area_style, 0);
    lv_style_set_bg_opa(&transparent_area_style, 0);
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
    lv_obj_set_style_text_font(tview->date_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->date_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    tabview_make_metric_clickable(tview->date_label, tview);
}
static void tabview_init_outdoor_section(tabview_t* tview, lv_obj_t* section_grid)
{
    lv_obj_t* outdoor_section = tabview_create_section(section_grid, 1, 0);
    lv_obj_set_grid_cell(outdoor_section, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_layout(outdoor_section, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_row(outdoor_section, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(outdoor_section, 12, LV_PART_MAIN | LV_STATE_DEFAULT);

    static lv_coord_t outdoor_col_dsc[] = { LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t outdoor_row_dsc[] = { LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT,
                                            LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(outdoor_section, outdoor_col_dsc, outdoor_row_dsc);

    lv_obj_t* temperature_icon_obj = lv_img_create(outdoor_section);
    lv_img_set_src(temperature_icon_obj, &temperature);
    lv_obj_set_grid_cell(temperature_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    tview->temp_outside_label = lv_label_create(outdoor_section);
    lv_label_set_text(tview->temp_outside_label, "--");
    lv_obj_set_style_text_font(tview->temp_outside_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->temp_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->temp_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    tabview_make_metric_clickable(tview->temp_outside_label, tview);

    lv_obj_t* feels_like_icon = lv_label_create(outdoor_section);
    lv_label_set_text(feels_like_icon, "T");
    lv_obj_set_style_text_font(feels_like_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(feels_like_icon, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    tview->feels_like_label = lv_label_create(outdoor_section);
    lv_label_set_text(tview->feels_like_label, "--");
    lv_obj_set_style_text_font(tview->feels_like_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->feels_like_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->feels_like_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    tabview_make_metric_clickable(tview->feels_like_label, tview);

    lv_obj_t* high_icon = lv_label_create(outdoor_section);
    lv_label_set_text(high_icon, LV_SYMBOL_UP);
    lv_obj_set_style_text_font(high_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(high_icon, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    tview->daily_high_label = lv_label_create(outdoor_section);
    lv_label_set_text(tview->daily_high_label, "--");
    lv_obj_set_style_text_font(tview->daily_high_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->daily_high_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->daily_high_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tabview_make_metric_clickable(tview->daily_high_label, tview);

    lv_obj_t* low_icon = lv_label_create(outdoor_section);
    lv_label_set_text(low_icon, LV_SYMBOL_DOWN);
    lv_obj_set_style_text_font(low_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(low_icon, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    tview->daily_low_label = lv_label_create(outdoor_section);
    lv_label_set_text(tview->daily_low_label, "--");
    lv_obj_set_style_text_font(tview->daily_low_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->daily_low_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->daily_low_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    tabview_make_metric_clickable(tview->daily_low_label, tview);

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
                                                   LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(outdoor_details_section, outdoor_details_col_dsc, outdoor_details_row_dsc);

    lv_obj_t* outdoor_title = lv_label_create(outdoor_details_section);
    lv_label_set_text(outdoor_title, "Outdoor");
    lv_obj_set_style_text_font(outdoor_title, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(outdoor_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(outdoor_title, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t* humidity_icon_obj = lv_img_create(outdoor_details_section);
    lv_img_set_src(humidity_icon_obj, &humidity_icon);
    lv_obj_set_grid_cell(humidity_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    tview->humidity_outside_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(tview->humidity_outside_label, "--%");
    lv_obj_set_style_text_font(tview->humidity_outside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->humidity_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->humidity_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tabview_make_metric_clickable(tview->humidity_outside_label, tview);

    lv_obj_t* pressure_icon_obj = lv_img_create(outdoor_details_section);
    lv_img_set_src(pressure_icon_obj, &pressure_icon);
    lv_obj_set_grid_cell(pressure_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    tview->pressure_outside_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(tview->pressure_outside_label, "--");
    lv_obj_set_style_text_font(tview->pressure_outside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->pressure_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->pressure_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tabview_make_metric_clickable(tview->pressure_outside_label, tview);

    lv_obj_t* wind_icon_obj = lv_img_create(outdoor_details_section);
    lv_img_set_src(wind_icon_obj, &wind_icon);
    lv_obj_set_grid_cell(wind_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    tview->wind_speed_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(tview->wind_speed_label, "--");
    lv_obj_set_style_text_font(tview->wind_speed_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->wind_speed_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->wind_speed_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    tabview_make_metric_clickable(tview->wind_speed_label, tview);

    lv_obj_t* precipitation_icon = lv_label_create(outdoor_details_section);
    lv_label_set_text(precipitation_icon, "Rain 24h");
    lv_obj_set_style_text_font(precipitation_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(precipitation_icon, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);

    tview->precipitation_outside_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(tview->precipitation_outside_label, "-- mm");
    lv_obj_set_style_text_font(tview->precipitation_outside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->precipitation_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->precipitation_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 4, 1);
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
    lv_obj_t* indoor_section = tabview_create_section(section_grid, 0, 1);
    lv_obj_set_grid_cell(indoor_section, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_set_layout(indoor_section, LV_LAYOUT_GRID);
    lv_obj_set_style_pad_row(indoor_section, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(indoor_section, 12, LV_PART_MAIN | LV_STATE_DEFAULT);
    static lv_coord_t indoor_col_dsc[] = { LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_CONTENT,
                                           LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t indoor_row_dsc[] = { LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT,
                                           LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(indoor_section, indoor_col_dsc, indoor_row_dsc);

    lv_obj_t* indoor_title = lv_label_create(indoor_section);
    lv_label_set_text(indoor_title, "Indoor");
    lv_obj_set_style_text_font(indoor_title, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(indoor_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(indoor_title, LV_GRID_ALIGN_CENTER, 0, 4, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t* temperature_icon_indoor = lv_img_create(indoor_section);
    lv_img_set_src(temperature_icon_indoor, &temperature);
    lv_obj_set_grid_cell(temperature_icon_indoor, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tview->temp_inside_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->temp_inside_label, "--");
    lv_obj_set_style_text_font(tview->temp_inside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->temp_inside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->temp_inside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tabview_make_metric_clickable(tview->temp_inside_label, tview);

    lv_obj_t* humidity_icon_indoor = lv_img_create(indoor_section);
    lv_img_set_src(humidity_icon_indoor, &humidity_icon);
    lv_obj_set_grid_cell(humidity_icon_indoor, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tview->humidity_inside_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->humidity_inside_label, "--%");
    lv_obj_set_style_text_font(tview->humidity_inside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->humidity_inside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->humidity_inside_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    tabview_make_metric_clickable(tview->humidity_inside_label, tview);

    lv_obj_t* pressure_icon_indoor = lv_img_create(indoor_section);
    lv_img_set_src(pressure_icon_indoor, &pressure_icon);
    lv_obj_set_grid_cell(pressure_icon_indoor, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tview->pressure_inside_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->pressure_inside_label, "--");
    lv_obj_set_style_text_font(tview->pressure_inside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->pressure_inside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->pressure_inside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tabview_make_metric_clickable(tview->pressure_inside_label, tview);

    lv_obj_t* voc_title_label = lv_label_create(indoor_section);
    lv_label_set_text(voc_title_label, "VOC");
    lv_obj_set_style_text_font(voc_title_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(voc_title_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(voc_title_label, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tview->voc_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->voc_label, "--");
    lv_obj_set_style_text_font(tview->voc_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->voc_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->voc_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    tabview_make_metric_clickable(tview->voc_label, tview);

    lv_obj_t* co2_icon_obj = lv_img_create(indoor_section);
    lv_img_set_src(co2_icon_obj, &co2);
    lv_obj_set_grid_cell(co2_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    tview->co2_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->co2_label, "--");
    lv_obj_set_style_text_font(tview->co2_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->co2_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->co2_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    tabview_make_metric_clickable(tview->co2_label, tview);

    lv_obj_t* iaq_icon_obj = lv_img_create(indoor_section);
    lv_img_set_src(iaq_icon_obj, &aqi);
    lv_obj_set_grid_cell(iaq_icon_obj, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    tview->iaq_label = lv_label_create(indoor_section);
    lv_label_set_text(tview->iaq_label, "--");
    lv_obj_set_style_text_font(tview->iaq_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(tview->iaq_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(tview->iaq_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 3, 1);
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
    tview->tab_bar = lv_tabview_get_tab_btns(tview->cont);
    lv_obj_set_style_pad_right(tview->tab_bar, LV_HOR_RES / 2, 0);
    return tview;
}
