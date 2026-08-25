#include "DashboardWidgets.h"

#include "entities/ui/components/common.h"

static lv_obj_t* createSection(lv_obj_t* section_grid, lv_coord_t col, lv_coord_t row)
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

static void navigateHistoryTabCb(lv_event_t* e)
{
    DashboardWidgets* widgets = static_cast<DashboardWidgets*>(lv_event_get_user_data(e));
    if (widgets == nullptr || widgets->cont == nullptr || widgets->tab_history == nullptr)
        return;

    uint32_t history_index = lv_obj_get_index(widgets->tab_history);
    lv_tabview_set_act(widgets->cont, history_index, LV_ANIM_ON);
}

// Clicking a metric label jumps to the history tab showing that metric's plot.
static void makeMetricClickable(lv_obj_t* obj, DashboardWidgets* widgets)
{
    if (obj == nullptr || widgets == nullptr)
        return;

    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, navigateHistoryTabCb, LV_EVENT_CLICKED, widgets);
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

static void initTimeSection(DashboardWidgets* widgets, lv_obj_t* section_grid)
{
    lv_obj_t* time_section = createSection(section_grid, 0, 0);
    lv_obj_set_layout(time_section, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(time_section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(time_section, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    widgets->label = lv_label_create(time_section);
    lv_label_set_text(widgets->label, "--:--");
    lv_obj_set_style_text_font(widgets->label, &saira_condensed_medium, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    makeMetricClickable(widgets->label, widgets);
    widgets->date_label = lv_label_create(time_section);
    lv_label_set_text(widgets->date_label, "--");
    lv_obj_set_style_text_font(widgets->date_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->date_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    makeMetricClickable(widgets->date_label, widgets);
}

static void initOutdoorSection(DashboardWidgets* widgets, lv_obj_t* section_grid)
{
    lv_obj_t* outdoor_section = createSection(section_grid, 1, 0);
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

    widgets->temp_outside_label = lv_label_create(outdoor_section);
    lv_label_set_text(widgets->temp_outside_label, "--");
    lv_obj_set_style_text_font(widgets->temp_outside_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->temp_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->temp_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    makeMetricClickable(widgets->temp_outside_label, widgets);

    lv_obj_t* feels_like_icon = lv_label_create(outdoor_section);
    lv_label_set_text(feels_like_icon, "T");
    lv_obj_set_style_text_font(feels_like_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(feels_like_icon, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    widgets->feels_like_label = lv_label_create(outdoor_section);
    lv_label_set_text(widgets->feels_like_label, "--");
    lv_obj_set_style_text_font(widgets->feels_like_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->feels_like_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->feels_like_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    makeMetricClickable(widgets->feels_like_label, widgets);

    lv_obj_t* high_icon = lv_label_create(outdoor_section);
    lv_label_set_text(high_icon, LV_SYMBOL_UP);
    lv_obj_set_style_text_font(high_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(high_icon, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    widgets->daily_high_label = lv_label_create(outdoor_section);
    lv_label_set_text(widgets->daily_high_label, "--");
    lv_obj_set_style_text_font(widgets->daily_high_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->daily_high_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->daily_high_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    makeMetricClickable(widgets->daily_high_label, widgets);

    lv_obj_t* low_icon = lv_label_create(outdoor_section);
    lv_label_set_text(low_icon, LV_SYMBOL_DOWN);
    lv_obj_set_style_text_font(low_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(low_icon, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    widgets->daily_low_label = lv_label_create(outdoor_section);
    lv_label_set_text(widgets->daily_low_label, "--");
    lv_obj_set_style_text_font(widgets->daily_low_label, &lv_font_montserrat_48, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->daily_low_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->daily_low_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    makeMetricClickable(widgets->daily_low_label, widgets);

}

static void initOutdoorDetailsSection(DashboardWidgets* widgets, lv_obj_t* section_grid)
{
    lv_obj_t* outdoor_details_section = createSection(section_grid, 1, 1);
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

    widgets->humidity_outside_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(widgets->humidity_outside_label, "--%");
    lv_obj_set_style_text_font(widgets->humidity_outside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->humidity_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->humidity_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    makeMetricClickable(widgets->humidity_outside_label, widgets);

    lv_obj_t* pressure_icon_obj = lv_img_create(outdoor_details_section);
    lv_img_set_src(pressure_icon_obj, &pressure_icon);
    lv_obj_set_grid_cell(pressure_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

    widgets->pressure_outside_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(widgets->pressure_outside_label, "--");
    lv_obj_set_style_text_font(widgets->pressure_outside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->pressure_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->pressure_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    makeMetricClickable(widgets->pressure_outside_label, widgets);

    lv_obj_t* wind_icon_obj = lv_img_create(outdoor_details_section);
    lv_img_set_src(wind_icon_obj, &wind_icon);
    lv_obj_set_grid_cell(wind_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);

    widgets->wind_speed_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(widgets->wind_speed_label, "--");
    lv_obj_set_style_text_font(widgets->wind_speed_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->wind_speed_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->wind_speed_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    makeMetricClickable(widgets->wind_speed_label, widgets);

    lv_obj_t* precipitation_icon = lv_label_create(outdoor_details_section);
    lv_label_set_text(precipitation_icon, "Rain 24h");
    lv_obj_set_style_text_font(precipitation_icon, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(precipitation_icon, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 4, 1);

    widgets->precipitation_outside_label = lv_label_create(outdoor_details_section);
    lv_label_set_text(widgets->precipitation_outside_label, "-- mm");
    lv_obj_set_style_text_font(widgets->precipitation_outside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->precipitation_outside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->precipitation_outside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 4, 1);
    makeMetricClickable(widgets->precipitation_outside_label, widgets);
}

static void initHistoryTab(DashboardWidgets* widgets)
{
    if (widgets == nullptr)
        return;

    widgets->tab_history = lv_tabview_add_tab(widgets->cont, "History");
    lv_obj_set_style_pad_top(widgets->tab_history, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(widgets->tab_history, 5, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* history_root = lv_obj_create(widgets->tab_history);
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

    widgets->bottom_plot_title = lv_label_create(history_root);
    lv_label_set_text(widgets->bottom_plot_title, "History");
    lv_obj_set_width(widgets->bottom_plot_title, LV_PCT(100));
    lv_obj_set_style_text_font(widgets->bottom_plot_title, &lv_font_montserrat_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->bottom_plot_title, LV_TEXT_ALIGN_CENTER,
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

    widgets->bottom_plot_chart = lv_chart_create(plot_container);
    lv_obj_set_size(widgets->bottom_plot_chart, LV_PCT(100), LV_PCT(100));
    lv_obj_set_grid_cell(widgets->bottom_plot_chart, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH,
                         0, 1);
    lv_obj_set_style_pad_all(widgets->bottom_plot_chart, 8, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(widgets->bottom_plot_chart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(widgets->bottom_plot_chart, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_chart_set_type(widgets->bottom_plot_chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(widgets->bottom_plot_chart, kBottomPlotSlotCount);
    lv_chart_set_range(widgets->bottom_plot_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_chart_set_axis_tick(widgets->bottom_plot_chart, LV_CHART_AXIS_PRIMARY_Y, 6, 3, 6, 2, true, 40);
    lv_chart_set_axis_tick(widgets->bottom_plot_chart, LV_CHART_AXIS_PRIMARY_X, 6, 3, 7, 4, true, 30);
    lv_obj_add_event_cb(widgets->bottom_plot_chart, bottom_plot_x_axis_label_event_cb,
                        LV_EVENT_DRAW_PART_BEGIN, nullptr);
    lv_obj_add_event_cb(widgets->bottom_plot_chart, bottom_plot_y_axis_label_event_cb,
                        LV_EVENT_DRAW_PART_BEGIN, nullptr);
    lv_obj_add_event_cb(widgets->bottom_plot_chart, bottom_plot_cursor_draw_event_cb,
                        LV_EVENT_DRAW_PART_BEGIN, nullptr);
    lv_obj_set_style_text_font(widgets->bottom_plot_chart, &lv_font_montserrat_12,
                               LV_PART_TICKS | LV_STATE_DEFAULT);
    widgets->bottom_plot_series = lv_chart_add_series(widgets->bottom_plot_chart,
                                                      lv_palette_main(LV_PALETTE_LIGHT_BLUE),
                                                      LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_all_value(widgets->bottom_plot_chart, widgets->bottom_plot_series, 0);
    widgets->bottom_plot_cursor =
        lv_chart_add_cursor(widgets->bottom_plot_chart, lv_color_white(), LV_DIR_VER);
    if (widgets->bottom_plot_cursor)
    {
        lv_point_t origin = { 0, 0 };
        lv_chart_set_cursor_pos(widgets->bottom_plot_chart, widgets->bottom_plot_cursor, &origin);
    }

    lv_obj_t* plot_bottom_spacer = lv_obj_create(plot_container);
    lv_obj_set_size(plot_bottom_spacer, LV_PCT(100), 24);
    lv_obj_clear_flag(plot_bottom_spacer, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(plot_bottom_spacer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(plot_bottom_spacer, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(plot_bottom_spacer, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
}

static void initIndoorSection(DashboardWidgets* widgets, lv_obj_t* section_grid)
{
    lv_obj_t* indoor_section = createSection(section_grid, 0, 1);
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
    widgets->temp_inside_label = lv_label_create(indoor_section);
    lv_label_set_text(widgets->temp_inside_label, "--");
    lv_obj_set_style_text_font(widgets->temp_inside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->temp_inside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->temp_inside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    makeMetricClickable(widgets->temp_inside_label, widgets);

    lv_obj_t* humidity_icon_indoor = lv_img_create(indoor_section);
    lv_img_set_src(humidity_icon_indoor, &humidity_icon);
    lv_obj_set_grid_cell(humidity_icon_indoor, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    widgets->humidity_inside_label = lv_label_create(indoor_section);
    lv_label_set_text(widgets->humidity_inside_label, "--%");
    lv_obj_set_style_text_font(widgets->humidity_inside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->humidity_inside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->humidity_inside_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);
    makeMetricClickable(widgets->humidity_inside_label, widgets);

    lv_obj_t* pressure_icon_indoor = lv_img_create(indoor_section);
    lv_img_set_src(pressure_icon_indoor, &pressure_icon);
    lv_obj_set_grid_cell(pressure_icon_indoor, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    widgets->pressure_inside_label = lv_label_create(indoor_section);
    lv_label_set_text(widgets->pressure_inside_label, "--");
    lv_obj_set_style_text_font(widgets->pressure_inside_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->pressure_inside_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->pressure_inside_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    makeMetricClickable(widgets->pressure_inside_label, widgets);

    lv_obj_t* voc_title_label = lv_label_create(indoor_section);
    lv_label_set_text(voc_title_label, "VOC");
    lv_obj_set_style_text_font(voc_title_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(voc_title_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(voc_title_label, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    widgets->voc_label = lv_label_create(indoor_section);
    lv_label_set_text(widgets->voc_label, "--");
    lv_obj_set_style_text_font(widgets->voc_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->voc_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->voc_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);
    makeMetricClickable(widgets->voc_label, widgets);

    lv_obj_t* co2_icon_obj = lv_img_create(indoor_section);
    lv_img_set_src(co2_icon_obj, &co2);
    lv_obj_set_grid_cell(co2_icon_obj, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    widgets->co2_label = lv_label_create(indoor_section);
    lv_label_set_text(widgets->co2_label, "--");
    lv_obj_set_style_text_font(widgets->co2_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->co2_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->co2_label, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    makeMetricClickable(widgets->co2_label, widgets);

    lv_obj_t* iaq_icon_obj = lv_img_create(indoor_section);
    lv_img_set_src(iaq_icon_obj, &aqi);
    lv_obj_set_grid_cell(iaq_icon_obj, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    widgets->iaq_label = lv_label_create(indoor_section);
    lv_label_set_text(widgets->iaq_label, "--");
    lv_obj_set_style_text_font(widgets->iaq_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->iaq_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_grid_cell(widgets->iaq_label, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 3, 1);
    makeMetricClickable(widgets->iaq_label, widgets);
}

static void initDashboardTab(DashboardWidgets* widgets)
{
    if (widgets == nullptr)
        return;
    widgets->tab_dashboard = lv_tabview_add_tab(widgets->cont, "Dashboard");
    lv_obj_set_style_pad_top(widgets->tab_dashboard, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(widgets->tab_dashboard, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t* section_grid = lv_obj_create(widgets->tab_dashboard);
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
    initTimeSection(widgets, section_grid);
    initOutdoorSection(widgets, section_grid);
    initOutdoorDetailsSection(widgets, section_grid);
    initIndoorSection(widgets, section_grid);

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

DashboardWidgets* dashboard_view_create(lv_obj_t* parent, int32_t tab_h)
{
    DashboardWidgets* widgets =
        static_cast<DashboardWidgets*>(heap_caps_malloc(sizeof(DashboardWidgets), MALLOC_CAP_SPIRAM));
    widgets->tab_h  = tab_h;
    widgets->parent = parent;
    widgets->cont   = lv_tabview_create(parent, LV_DIR_TOP, widgets->tab_h);
    initDashboardTab(widgets);
    initHistoryTab(widgets);
    widgets->tab_settings = lv_tabview_add_tab(widgets->cont, "Settings");
    lv_obj_set_style_pad_top(widgets->tab_settings, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(widgets->tab_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    widgets->tab_bar = lv_tabview_get_tab_btns(widgets->cont);
    lv_obj_set_style_pad_right(widgets->tab_bar, LV_HOR_RES / 2, 0);
    return widgets;
}
