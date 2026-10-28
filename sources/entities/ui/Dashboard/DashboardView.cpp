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

static void navigateStatusTabCb(lv_event_t* e)
{
    DashboardWidgets* widgets = static_cast<DashboardWidgets*>(lv_event_get_user_data(e));
    if (widgets == nullptr || widgets->cont == nullptr || widgets->tab_status == nullptr)
        return;

    uint32_t status_index = lv_obj_get_index(widgets->tab_status);
    lv_tabview_set_act(widgets->cont, status_index, LV_ANIM_ON);
}

// Clicking a dashboard metric opens the sensor status tab.
static void makeMetricClickable(lv_obj_t* obj, DashboardWidgets* widgets)
{
    if (obj == nullptr || widgets == nullptr)
        return;

    lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(obj, navigateStatusTabCb, LV_EVENT_CLICKED, widgets);
}

static void initTimeSection(DashboardWidgets* widgets, lv_obj_t* section_grid)
{
    lv_obj_t* time_section = createSection(section_grid, 0, 0);
    lv_obj_set_grid_cell(time_section, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_style_pad_row(time_section, 4, 0);
    lv_obj_set_layout(time_section, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(time_section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(time_section, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    widgets->label = lv_label_create(time_section);
    lv_label_set_text(widgets->label, "--:--");
    lv_obj_set_style_text_font(widgets->label, &saira_condensed_medium, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    widgets->date_label = lv_label_create(time_section);
    lv_label_set_text(widgets->date_label, "--");
    lv_obj_set_style_text_font(widgets->date_label, &lv_font_montserrat_28, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(widgets->date_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
}

static lv_obj_t* createTemperatureSection(lv_obj_t* grid, lv_coord_t col,
                                           lv_coord_t row, const char* title,
                                           lv_palette_t accent)
{
    lv_obj_t* section = createSection(grid, col, row);
    lv_obj_set_flex_flow(section, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(section, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(section, 8, 0);
    lv_obj_t* heading = lv_label_create(section);
    lv_label_set_text(heading, title);
    lv_obj_set_style_text_font(heading, &lv_font_montserrat_20, 0);
    lv_obj_t* value = lv_label_create(section);
    lv_label_set_text(value, "--");
    lv_obj_set_style_text_font(value, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(value, lv_palette_lighten(accent, 2), 0);
    return value;
}

static void initStatusTab(DashboardWidgets* widgets)
{
    widgets->tab_status = lv_tabview_add_tab(widgets->cont, "Status");
    lv_obj_set_style_pad_all(widgets->tab_status, 10, 0);
    lv_obj_set_style_pad_row(widgets->tab_status, 4, 0);
    lv_obj_set_flex_flow(widgets->tab_status, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(widgets->tab_status, LV_DIR_VER);

    lv_obj_t* heading = lv_label_create(widgets->tab_status);
    lv_label_set_text(heading, "Sensor status  |  Today's history: 00:00 - 24:00");
    lv_obj_set_style_text_font(heading, &lv_font_montserrat_16, 0);

    static const char* titles[] = { "Temperature", "Humidity", "Pressure", "CO2", "VOC", "IAQ" };
    static lv_coord_t cols[] = { LV_GRID_FR(3), LV_GRID_FR(3), LV_GRID_FR(5), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t rows[] = { LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    for (size_t i = 0; i < 6; ++i)
    {
        lv_obj_t* row = lv_obj_create(widgets->tab_status);
        lv_obj_set_size(row, LV_PCT(100), 62);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(row, 6, 0);
        lv_obj_set_style_pad_column(row, 10, 0);
        lv_obj_set_grid_dsc_array(row, cols, rows);

        lv_obj_t* title = lv_label_create(row);
        lv_label_set_text(title, titles[i]);
        lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
        lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        auto& metric = widgets->status_metrics[i];
        metric.value = lv_label_create(row);
        lv_label_set_text(metric.value, "--");
        lv_obj_set_style_text_font(metric.value, &lv_font_montserrat_20, 0);
        lv_obj_set_grid_cell(metric.value, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        metric.chart = lv_chart_create(row);
        lv_obj_set_grid_cell(metric.chart, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
        lv_obj_clear_flag(metric.chart, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(metric.chart, 2, 0);
        lv_obj_set_style_bg_opa(metric.chart, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(metric.chart, 0, 0);
        lv_obj_set_style_size(metric.chart, 2, LV_PART_INDICATOR);
        lv_chart_set_type(metric.chart, LV_CHART_TYPE_LINE);
        lv_chart_set_point_count(metric.chart, 48);
        lv_chart_set_div_line_count(metric.chart, 2, 3);
        metric.series = lv_chart_add_series(metric.chart, lv_palette_main(LV_PALETTE_LIGHT_BLUE),
                                            LV_CHART_AXIS_PRIMARY_Y);
        lv_chart_set_all_value(metric.chart, metric.series, LV_CHART_POINT_NONE);
    }
}

static void initDashboardTab(DashboardWidgets* widgets)
{
    widgets->tab_dashboard = lv_tabview_add_tab(widgets->cont, "Dashboard");
    lv_obj_clear_flag(widgets->tab_dashboard, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(widgets->tab_dashboard, 10, 0);
    lv_obj_t* grid = lv_obj_create(widgets->tab_dashboard);
    lv_obj_set_size(grid, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(grid, 0, 0);
    lv_obj_set_style_pad_all(grid, 0, 0);
    lv_obj_set_style_pad_row(grid, 6, 0);
    lv_obj_set_style_pad_column(grid, 20, 0);
    static lv_coord_t cols[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t rows[] = { 200, LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_grid_dsc_array(grid, cols, rows);
    initTimeSection(widgets, grid);
    widgets->temp_outside_label = createTemperatureSection(grid, 0, 1, "Outside now", LV_PALETTE_LIGHT_BLUE);
    widgets->temp_inside_label = createTemperatureSection(grid, 1, 1, "Inside now", LV_PALETTE_ORANGE);
    makeMetricClickable(widgets->temp_inside_label, widgets);
    widgets->daily_low_label = createTemperatureSection(grid, 0, 2, "Next night / low", LV_PALETTE_LIGHT_BLUE);
    widgets->daily_high_label = createTemperatureSection(grid, 1, 2, "Following day / high", LV_PALETTE_ORANGE);
}

DashboardWidgets* dashboard_view_create(lv_obj_t* parent, int32_t tab_h)
{
    DashboardWidgets* widgets =
        static_cast<DashboardWidgets*>(heap_caps_calloc(1, sizeof(DashboardWidgets), MALLOC_CAP_SPIRAM));
    widgets->tab_h  = tab_h;
    widgets->parent = parent;
    widgets->cont   = lv_tabview_create(parent, LV_DIR_TOP, widgets->tab_h);
    initDashboardTab(widgets);
    initStatusTab(widgets);
    // These readings remain on the Status tab after simplifying the dashboard.
    widgets->humidity_inside_label = widgets->status_metrics[1].value;
    widgets->pressure_inside_label = widgets->status_metrics[2].value;
    widgets->co2_label = widgets->status_metrics[3].value;
    widgets->voc_label = widgets->status_metrics[4].value;
    widgets->iaq_label = widgets->status_metrics[5].value;
    widgets->tab_settings = lv_tabview_add_tab(widgets->cont, "Settings");
    lv_obj_set_style_pad_top(widgets->tab_settings, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(widgets->tab_settings, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
    widgets->tab_bar = lv_tabview_get_tab_btns(widgets->cont);
    lv_obj_set_style_pad_right(widgets->tab_bar, LV_HOR_RES / 2, 0);
    return widgets;
}
