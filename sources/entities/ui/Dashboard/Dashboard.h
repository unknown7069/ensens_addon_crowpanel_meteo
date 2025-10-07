#pragma once

#include "esp_log.h"
#include "esp_log_level.h"
#include "lvgl.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

#include "Aggregator.h"
#include "entities/EnvironmentalSensorData.h"
#include "entities/ui/components/common.h"
#include "entities/Units.h"
#include "entities/ui/components/pressure_box.h"
#include "entities/ui/sensors_settings/SensorSettings.h"

#include "entities/ui/wifi_screen/WifiScreen.h"

#include "adapters/lvgl/lvgl_port_v8.h"
#include "settings.h"
#include <cmath>

#include <iterator>
#include <sstream>
#include <initializer_list>
#include <utility>

#define TENDENCY_VALUES_NUM 5
#define TENDENCY_VALUES_MAX_ANGLE 90
#define TENDENCY_VALUES_STEP_ANGLE (TENDENCY_VALUES_MAX_ANGLE / (TENDENCY_VALUES_NUM - 1))

LV_IMG_DECLARE(settings);

class Dashboard
{
    static constexpr char TAG[] = "Dashboard";
    tabview_t*            tv_;
    lv_obj_t*             cont_;
    default_box_t*        iaq_box_;
    default_box_t*        co2_box_;
    default_box_t*        voc_box_;
    data_box_t*           temp_box_;
    humi_dew_data_box_t*  humi_dew_point_box_;
    lv_obj_t*             empty_label_;
    pressure_box_t*       pressure_box_;
    SensorSettings*       sensor_settings_;
    days_header_t*        days_header_;
    TimeZoneLabel         timeZoneLabel;
    TimeZoneLabel         dateLabel;
    Button                settingsButton;
    Image                 configIcon;

    struct {
        float temperature;
        float humidity;
        float dew_point;
        float pressure;
        float co2;
        float voc;
        float iaq;
    } current_data_;

    struct Y_axis_info {
        UnitType                 mode                 = UnitType::hPa;
        uint16_t                 count                = 7;
        std::vector<const char*> pressure_hPa_labels  = { " 1100", " 1066", " 1033", " 1000",
                                                          " 966",  " 933",  " 900" };
        std::vector<const char*> pressure_mmHg_labels = { " 800", " 783", " 766", " 750",
                                                          " 733", " 716", " 700" };
    } Y_axis_info_;

    static void configSettingsButtonCallback(lv_event_t* e, void* context)
    {
        if (lv_event_get_code(e) == LV_EVENT_CLICKED)
        {
            ESP_LOGD(TAG, "settingsButton pressed");
            WifiScreen::instance().load();
        }
    }

    Dashboard()                            = default;
    Dashboard(const Dashboard&)            = delete;
    Dashboard& operator=(const Dashboard&) = delete;

    void lock()
    {
        lvgl_port_lock(-1);
    }

    void unlock()
    {
        lvgl_port_unlock();
    }

    static void draw_event_cb_x_axis(lv_event_t* e)
    {
        lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
        if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL))
            return;

        const char** labels = (const char**)lv_event_get_user_data(e);
        if (labels)
        {
            if (dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->text)
            {
                lv_snprintf(dsc->text, dsc->text_length, "%s", labels[dsc->value]);
            }
        }
    }

    static void draw_event_cb_y_axis(lv_event_t* e)
    {
        lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
        if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_TICK_LABEL))
            return;

        Y_axis_info* info = (Y_axis_info*)lv_event_get_user_data(e);
        if (info)
        {
            static int16_t idx_label = 0;
            if (dsc->id == LV_CHART_AXIS_SECONDARY_Y && dsc->text)
            {
                idx_label = (idx_label == info->count) ? 0 : idx_label;
                switch (info->mode)
                {
                case UnitType::hPa:
                    lv_snprintf(dsc->text, dsc->text_length, "%s",
                                info->pressure_hPa_labels[idx_label++]);
                    break;
                case UnitType::mmHg:
                    lv_snprintf(dsc->text, dsc->text_length, "%s",
                                info->pressure_mmHg_labels[idx_label++]);
                    break;

                default:
                    break;
                }
            }
        }
    }

    void settings_button_init()
    {
        /* settingsButton */
        settingsButton.create(cont_);
        settingsButton.setEventCallback(configSettingsButtonCallback);
        {
            // create image.
            configIcon.create(settingsButton.get());
            configIcon.scale(0.75f);
            configIcon.set(&settings);
            configIcon.align(LV_ALIGN_TOP_RIGHT);
        }
    }

    void iaq_box_init()
    {
        iaq_box_ = default_box_create(cont_);
        lv_meter_set_scale_range(iaq_box_->gauge->meter, iaq_box_->gauge->scale, 0, 500, 270, 90);

        lv_meter_indicator_t* indic;
        indic = lv_meter_add_arc(iaq_box_->gauge->meter, iaq_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_GREEN), 0);
        lv_meter_set_indicator_start_value(iaq_box_->gauge->meter, indic, 0);
        lv_meter_set_indicator_end_value(iaq_box_->gauge->meter, indic, 50);

        indic = lv_meter_add_arc(iaq_box_->gauge->meter, iaq_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_YELLOW), 0);
        lv_meter_set_indicator_start_value(iaq_box_->gauge->meter, indic, 50);
        lv_meter_set_indicator_end_value(iaq_box_->gauge->meter, indic, 100);

        indic = lv_meter_add_arc(iaq_box_->gauge->meter, iaq_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_ORANGE), 0);
        lv_meter_set_indicator_start_value(iaq_box_->gauge->meter, indic, 100);
        lv_meter_set_indicator_end_value(iaq_box_->gauge->meter, indic, 150);

        indic = lv_meter_add_arc(iaq_box_->gauge->meter, iaq_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_RED), 0);
        lv_meter_set_indicator_start_value(iaq_box_->gauge->meter, indic, 150);
        lv_meter_set_indicator_end_value(iaq_box_->gauge->meter, indic, 200);

        indic = lv_meter_add_arc(iaq_box_->gauge->meter, iaq_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_PURPLE), 0);
        lv_meter_set_indicator_start_value(iaq_box_->gauge->meter, indic, 200);
        lv_meter_set_indicator_end_value(iaq_box_->gauge->meter, indic, 300);

        indic = lv_meter_add_arc(iaq_box_->gauge->meter, iaq_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_DEEP_PURPLE), 0);
        lv_meter_set_indicator_start_value(iaq_box_->gauge->meter, indic, 300);
        lv_meter_set_indicator_end_value(iaq_box_->gauge->meter, indic, 500);

        lv_label_set_text(iaq_box_->title, "IAQ");
        lv_label_set_text(iaq_box_->value->int_part, "-");
        lv_label_set_text(iaq_box_->dimension, "");
        lv_meter_set_indicator_value(iaq_box_->gauge->meter, iaq_box_->gauge->needle, 50);
        lv_meter_set_indicator_value(iaq_box_->tend->meter, iaq_box_->tend->needle, 90);
    }

    void co2_box_init()
    {
        co2_box_ = default_box_create(cont_);
        lv_meter_set_scale_range(co2_box_->gauge->meter, co2_box_->gauge->scale, 0, 10000, 270, 90);

        lv_meter_indicator_t* indic;
        indic = lv_meter_add_arc(co2_box_->gauge->meter, co2_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_BLUE), 0);
        lv_meter_set_indicator_start_value(co2_box_->gauge->meter, indic, 0);
        lv_meter_set_indicator_end_value(co2_box_->gauge->meter, indic, 400);

        indic = lv_meter_add_arc(co2_box_->gauge->meter, co2_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_GREEN), 0);
        lv_meter_set_indicator_start_value(co2_box_->gauge->meter, indic, 400);
        lv_meter_set_indicator_end_value(co2_box_->gauge->meter, indic, 1000);

        indic = lv_meter_add_arc(co2_box_->gauge->meter, co2_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_YELLOW), 0);
        lv_meter_set_indicator_start_value(co2_box_->gauge->meter, indic, 1000);
        lv_meter_set_indicator_end_value(co2_box_->gauge->meter, indic, 2000);

        indic = lv_meter_add_arc(co2_box_->gauge->meter, co2_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_ORANGE), 0);
        lv_meter_set_indicator_start_value(co2_box_->gauge->meter, indic, 2000);
        lv_meter_set_indicator_end_value(co2_box_->gauge->meter, indic, 5000);

        indic = lv_meter_add_arc(co2_box_->gauge->meter, co2_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_RED), 0);
        lv_meter_set_indicator_start_value(co2_box_->gauge->meter, indic, 5000);
        lv_meter_set_indicator_end_value(co2_box_->gauge->meter, indic, 10000);

        lv_label_set_text(co2_box_->title, "CO2");
        lv_label_set_text(co2_box_->value->int_part, "-");
        lv_label_set_text(co2_box_->dimension, "ppm");
        lv_meter_set_indicator_value(co2_box_->gauge->meter, co2_box_->gauge->needle, 400);
        lv_meter_set_indicator_value(co2_box_->tend->meter, co2_box_->tend->needle, 90);
    }

    void voc_box_init()
    {
        voc_box_ = default_box_create(cont_);
        lv_meter_set_scale_range(voc_box_->gauge->meter, voc_box_->gauge->scale, 0, 5500, 270, 90);

        lv_meter_indicator_t* indic;
        indic = lv_meter_add_arc(voc_box_->gauge->meter, voc_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_GREEN), 0);
        lv_meter_set_indicator_start_value(voc_box_->gauge->meter, indic, 0);
        lv_meter_set_indicator_end_value(voc_box_->gauge->meter, indic, 65);

        indic = lv_meter_add_arc(voc_box_->gauge->meter, voc_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_LIGHT_GREEN), 0);
        lv_meter_set_indicator_start_value(voc_box_->gauge->meter, indic, 65);
        lv_meter_set_indicator_end_value(voc_box_->gauge->meter, indic, 220);

        indic = lv_meter_add_arc(voc_box_->gauge->meter, voc_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_YELLOW), 0);
        lv_meter_set_indicator_start_value(voc_box_->gauge->meter, indic, 220);
        lv_meter_set_indicator_end_value(voc_box_->gauge->meter, indic, 660);

        indic = lv_meter_add_arc(voc_box_->gauge->meter, voc_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_ORANGE), 0);
        lv_meter_set_indicator_start_value(voc_box_->gauge->meter, indic, 660);
        lv_meter_set_indicator_end_value(voc_box_->gauge->meter, indic, 2200);

        indic = lv_meter_add_arc(voc_box_->gauge->meter, voc_box_->gauge->scale, 10,
                                 lv_palette_main(LV_PALETTE_RED), 0);
        lv_meter_set_indicator_start_value(voc_box_->gauge->meter, indic, 2200);
        lv_meter_set_indicator_end_value(voc_box_->gauge->meter, indic, 5500);

        lv_label_set_text(voc_box_->title, "VOC");
        lv_label_set_text(voc_box_->value->int_part, "-");
        lv_label_set_text(voc_box_->dimension, "ppb");

        lv_meter_set_indicator_value(voc_box_->gauge->meter, voc_box_->gauge->needle, 300);
        lv_meter_set_indicator_value(voc_box_->tend->meter, voc_box_->tend->needle, 90);
    }

public:
    static Dashboard& instance()
    {
        static Dashboard instance;
        return instance;
    }

    tabview_t* getTabView()
    {
        return tv_;
    }

    void create_main_elements(lv_obj_t* parent)
    {
        cont_ = lv_obj_create(parent);
        lv_obj_set_size(cont_, lv_pct(100), lv_pct(100));
        lv_obj_clear_flag(cont_, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_layout(cont_, LV_LAYOUT_GRID);
        lv_obj_set_style_pad_left(cont_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_right(cont_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_top(cont_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_bottom(cont_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_row(cont_, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_pad_column(cont_, 5, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_bg_opa(cont_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(cont_, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        timeZoneLabel.create(cont_, &lv_font_montserrat_30);
        lv_obj_add_flag(timeZoneLabel.get(), LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_align(timeZoneLabel.get(), LV_ALIGN_TOP_LEFT, 0, 0); // 4

        dateLabel.create(cont_, &lv_font_montserrat_20);
        lv_obj_add_flag(dateLabel.get(), LV_OBJ_FLAG_IGNORE_LAYOUT);
        lv_obj_align(dateLabel.get(), LV_ALIGN_TOP_LEFT, 0, 37);

        temp_box_ = data_box_create(cont_);
        lv_label_set_text(temp_box_->value->int_part, "-");
        lv_obj_set_style_text_font(temp_box_->value->int_part, &lv_font_montserrat_30,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(temp_box_->value->unit_part, "°C");

        humi_dew_point_box_ = humi_dew_box_create(cont_);
        lv_label_set_text(humi_dew_point_box_->value_humi->int_part, "-");
        lv_obj_set_style_text_font(humi_dew_point_box_->value_humi->int_part,
                                   &lv_font_montserrat_30, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(humi_dew_point_box_->value_humi->unit_part, "\ %");

        lv_label_set_text(humi_dew_point_box_->value_dew->int_part, "-");
        lv_obj_set_style_text_font(humi_dew_point_box_->value_dew->int_part, &lv_font_montserrat_30,
                                   LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_label_set_text(humi_dew_point_box_->value_dew->unit_part, "°C");
        lv_label_set_text(humi_dew_point_box_->title_dew, "DEW POINT");

        pressure_box_ = pressure_box_create(cont_);

        iaq_box_init();
        co2_box_init();
        voc_box_init();
        settings_button_init();

        days_header_ = days_header_create(cont_);

        static const char* day_labels[] = { "-24", "-20", "-16", "-12", "-8", "-4", "0H" };
        lv_obj_add_event_cb(pressure_box_->plot.chart, draw_event_cb_x_axis,
                            LV_EVENT_DRAW_PART_BEGIN, day_labels);

        lv_obj_add_event_cb(pressure_box_->plot.chart, draw_event_cb_y_axis,
                            LV_EVENT_DRAW_PART_BEGIN, (void*)(&Y_axis_info_));

        lv_chart_set_point_count(pressure_box_->plot.chart, HISTORY_SIZE);

        /*Create a MAIN container with grid*/
        static lv_coord_t col_dsc[] = { 135, 135, 150, 150, 135, 50, LV_GRID_TEMPLATE_LAST };
        static lv_coord_t row_dsc[] = { 60, 70, LV_GRID_CONTENT, LV_GRID_CONTENT,
                                        LV_GRID_TEMPLATE_LAST };

        lv_obj_set_grid_dsc_array(cont_, col_dsc, row_dsc);

        lv_obj_set_grid_cell(days_header_->cont, LV_GRID_ALIGN_STRETCH, 1, 3, LV_GRID_ALIGN_STRETCH,
                             0, 1);
        lv_obj_set_grid_cell(iaq_box_->cont, LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_STRETCH, 0,
                             2);
        lv_obj_set_grid_cell(settingsButton.get(), LV_GRID_ALIGN_CENTER, 5, 1,
                             LV_GRID_ALIGN_STRETCH, 0, 4);
        lv_obj_set_grid_cell(pressure_box_->cont, LV_GRID_ALIGN_STRETCH, 2, 2,
                             LV_GRID_ALIGN_STRETCH, 1, 3);
        lv_obj_set_grid_cell(co2_box_->cont, LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_STRETCH, 2,
                             1);
        lv_obj_set_grid_cell(temp_box_->cont, LV_GRID_ALIGN_STRETCH, 0, 2, LV_GRID_ALIGN_STRETCH, 1,
                             2);

        lv_obj_set_grid_cell(humi_dew_point_box_->cont, LV_GRID_ALIGN_STRETCH, 0, 2,
                             LV_GRID_ALIGN_STRETCH, 3, 1);
        lv_obj_set_grid_cell(voc_box_->cont, LV_GRID_ALIGN_STRETCH, 4, 1, LV_GRID_ALIGN_STRETCH, 3,
                             1);
    }

    lv_obj_t* create(SensorSettings* sensor_settings, lv_obj_t* parent);

    void updateTemperature(const std::string& dev_name, const float value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        char str[16];
        snprintf(str, sizeof(str), "%.1f", value);
        const char* integer_part    = strtok(str, ".");
        const char* fractional_part = strtok(nullptr, "");

        if (fractional_part == nullptr)
        {
            fractional_part = "0";
        }

        lv_label_set_text_fmt(temp_box_->value->int_part, "%s", integer_part);
        if (tv_ && tv_->temp_inside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->temp_inside_label, "--");
            } else
            {
                const char* unit = unit_names.at(sensor_settings_->temperature);
                lv_label_set_text_fmt(tv_->temp_inside_label, "%+.1f%s", value, unit);
            }
        }
        unlock();
    }

    void updateTemperatureTendency(uint8_t value, int8_t dir)
    {
        lock();
        if (value <= TENDENCY_VALUES_NUM)
        {
            uint8_t angle = value * TENDENCY_VALUES_STEP_ANGLE;
            if (dir > 0)
            {
                angle = 90 + angle;
            } else
            {
                angle = 90 - angle;
            }
            ESP_LOGD(TAG, "temp tend value=%u, angle=%u", value, angle);
            lv_meter_set_indicator_value(temp_box_->tend->meter, temp_box_->tend->needle, angle);
        }
        unlock();
    }

    void updateOutsideTemperature(float value)
    {
        lock();
        if (tv_ && tv_->temp_outside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->temp_outside_label, "--");
            } else
            {
                float converted = convertValueToUnit(sensor_settings_->temperature, value);
                const char* unit = unit_names.at(sensor_settings_->temperature);
                lv_label_set_text_fmt(tv_->temp_outside_label, "%+.1f%s", converted, unit);
            }
        }
        unlock();
    }

    void updateOutsidePressure(float value)
    {
        lock();
        if (tv_ && tv_->pressure_outside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->pressure_outside_label, "--");
            } else
            {
                float converted = convertValueToDefault(UnitType::hPa, value);
                converted = convertValueToUnit(sensor_settings_->pressure, converted);
                const char* unit = unit_names.at(sensor_settings_->pressure);
                lv_label_set_text_fmt(tv_->pressure_outside_label, "%.0f%s", converted, unit);
            }
        }
        unlock();
    }

    void updateOutsideWindSpeed(float value)
    {
        lock();
        if (tv_ && tv_->wind_speed_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->wind_speed_label, "--");
            } else
            {
                lv_label_set_text_fmt(tv_->wind_speed_label, "%.1f m/s", value);
            }
        }
        unlock();
    }
    void updateOutsideHumidity(float value)
    {
        lock();
        if (tv_ && tv_->humidity_outside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->humidity_outside_label, "--%");
            } else
            {
                lv_label_set_text_fmt(tv_->humidity_outside_label, "%.0f%%", value);
            }
        }
        unlock();
    }

    void updateHumidity(const std::string& dev_name, const float value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        char str[16];
        snprintf(str, sizeof(str), "%.1f", value);
        const char* integer_part    = strtok(str, ".");
        const char* fractional_part = strtok(nullptr, "");

        if (fractional_part == nullptr)
        {
            fractional_part = "0";
        }

        lv_label_set_text_fmt(humi_dew_point_box_->value_humi->int_part, "%s", integer_part);
        if (tv_ && tv_->humidity_inside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->humidity_inside_label, "--%");
            } else
            {
                lv_label_set_text_fmt(tv_->humidity_inside_label, "%.0f%%", value);
            }
        }
        unlock();
    }

    void updateHumidityTendency(uint8_t value, int8_t dir)
    {
        lock();
        if (value <= TENDENCY_VALUES_NUM)
        {
            uint8_t angle = value * TENDENCY_VALUES_STEP_ANGLE;
            if (dir > 0)
            {
                angle = 90 + angle;
            } else
            {
                angle = 90 - angle;
            }
            ESP_LOGD(TAG, "humi tend value=%u, angle=%u", value, angle);
            lv_meter_set_indicator_value(temp_box_->tend->meter, temp_box_->tend->needle, angle);
        }
        unlock();
    }

    void updateDewPoint(const std::string& dev_name)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        static const float a = 17.27f;
        static const float b = 237.7f;

        char str[16];

        EnvironmentalSensor::DataSample<float> temperature_data;
        Aggregator::instance().getTemperatureData(dev_name, temperature_data);
        convertToDefault(temperature_data);
        float humidity = Aggregator::instance().getHumidityValue(dev_name);

        const float f_T_RH =
            (a * temperature_data.value) / (b + temperature_data.value) + log(humidity / 100);
        float dew_point_value = (b * f_T_RH) / (a - f_T_RH);
        dew_point_value       = convertValueToUnit(sensor_settings_->temperature, dew_point_value);

        ESP_LOGD(TAG, "temperature=%f, humidity=%f, dew=%f", temperature_data.value, humidity,
                 dew_point_value);

        if (!std::isnan(dew_point_value))
        {
            snprintf(str, sizeof(str), "%.1f", dew_point_value);
            const char* integer_part    = strtok(str, ".");
            const char* fractional_part = strtok(nullptr, "");

            if (fractional_part == nullptr)
                fractional_part = "0";

            lv_label_set_text_fmt(humi_dew_point_box_->value_dew->int_part, "%s", integer_part);
        }
        unlock();
    }

    void updatePressure(const std::string& dev_name, const float value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        char str[16];
        snprintf(str, sizeof(str), "%.1f", value);
        const char* integer_part    = strtok(str, ".");
        const char* fractional_part = strtok(nullptr, "");

        if (fractional_part == nullptr)
        {
            fractional_part = "0";
        }

        lv_label_set_text_fmt(pressure_box_->value->int_part, "%s", integer_part);
        if (tv_ && tv_->pressure_inside_label)
        {
            if (std::isnan(value))
            {
                lv_label_set_text(tv_->pressure_inside_label, "--");
            } else
            {
                const char* unit = unit_names.at(sensor_settings_->pressure);
                lv_label_set_text_fmt(tv_->pressure_inside_label, "%.0f%s", value, unit);
            }
        }
        lv_meter_set_indicator_value(pressure_box_->gauge->meter, pressure_box_->gauge->needle, value);
        unlock();
    }

    void setPressurePlotValue(int16_t value, uint16_t id)
    {
        lock();
        lv_chart_set_value_by_id(pressure_box_->plot.chart, pressure_box_->plot.y_sec, id, value);
        unlock();
    }
    void updatePressurePlot(const float* values, size_t num)
    {
        lock();
        for (size_t i = 0; i < num; i++)
        {
            lv_chart_set_next_value(pressure_box_->plot.chart, pressure_box_->plot.y_sec,
                                    static_cast<lv_coord_t>(values[i] * MULT_COEFF));
        }
        unlock();
    }
    void updatePressureTendency(uint8_t value, int8_t dir)
    {
        lock();
        if (value <= TENDENCY_VALUES_NUM)
        {
            uint8_t angle = value * TENDENCY_VALUES_STEP_ANGLE;
            if (dir > 0)
            {
                angle = 90 + angle;
            } else
            {
                angle = 90 - angle;
            }
            ESP_LOGD(TAG, "pressure tend value=%u, angle=%u", value, angle);
            lv_meter_set_indicator_value(pressure_box_->tend->meter, pressure_box_->tend->needle,
                                         angle);
        }
        unlock();
    }
    void updateCO2(const std::string& dev_name, const uint16_t value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        lv_label_set_text_fmt(co2_box_->value->int_part, "%d", value);
        lv_meter_set_indicator_value(co2_box_->gauge->meter, co2_box_->gauge->needle, value);
        if (tv_ && tv_->co2_label)
        {
            auto color = get_quality_color(value, { { 800, LV_PALETTE_GREEN }, { 1200, LV_PALETTE_YELLOW }, { 1600, LV_PALETTE_ORANGE } });
            lv_obj_set_style_text_color(tv_->co2_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_fmt(tv_->co2_label, "%u ppm", value);
        }
        unlock();
    }

    void updateCO2Tendency(uint8_t value, int8_t dir)
    {
        lock();
        if (value <= TENDENCY_VALUES_NUM)
        {
            uint8_t angle = value * TENDENCY_VALUES_STEP_ANGLE;
            if (dir > 0)
            {
                angle = 90 + angle;
            } else
            {
                angle = 90 - angle;
            }
            ESP_LOGD(TAG, "co2 tend value=%u, angle=%u", value, angle);
            lv_meter_set_indicator_value(co2_box_->tend->meter, co2_box_->tend->needle, angle);
        }
        unlock();
    }
    void updateVOC(const std::string& dev_name, const uint16_t value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        lv_label_set_text_fmt(voc_box_->value->int_part, "%d", value);
        lv_meter_set_indicator_value(voc_box_->gauge->meter, voc_box_->gauge->needle, value);
        if (tv_ && tv_->voc_label)
        {
            auto color = get_quality_color(value, { { 200, LV_PALETTE_GREEN }, { 400, LV_PALETTE_YELLOW }, { 1000, LV_PALETTE_ORANGE } });
            lv_obj_set_style_text_color(tv_->voc_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_fmt(tv_->voc_label, "%u ppb", value);
        }
        unlock();
    }

    void updateVOCTendency(uint8_t value, int8_t dir)
    {
        lock();
        if (value <= TENDENCY_VALUES_NUM)
        {
            uint8_t angle = value * TENDENCY_VALUES_STEP_ANGLE;
            if (dir > 0)
            {
                angle = 90 + angle;
            } else
            {
                angle = 90 - angle;
            }
            ESP_LOGD(TAG, "voc tend value=%u, angle=%u", value, angle);
            lv_meter_set_indicator_value(voc_box_->tend->meter, voc_box_->tend->needle, angle);
        }
        unlock();
    }
    void updateIAQ(const std::string& dev_name, const uint16_t value)
    {
        lock();
        if (dev_name != sensor_settings_->sensor_name)
        {
            unlock();
            return;
        }

        lv_label_set_text_fmt(iaq_box_->value->int_part, "%d", value);
        lv_meter_set_indicator_value(iaq_box_->gauge->meter, iaq_box_->gauge->needle, value);
        if (tv_ && tv_->iaq_label)
        {
            auto color = get_quality_color(value, { { 50, LV_PALETTE_GREEN }, { 100, LV_PALETTE_YELLOW }, { 150, LV_PALETTE_ORANGE } });
            lv_obj_set_style_text_color(tv_->iaq_label, color, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_label_set_text_fmt(tv_->iaq_label, "%u", value);
        }
        unlock();
    }

    void updateIAQTendency(uint8_t value, int8_t dir)
    {
        lock();
        if (value <= TENDENCY_VALUES_NUM)
        {
            uint8_t angle = value * TENDENCY_VALUES_STEP_ANGLE;
            if (dir > 0)
            {
                angle = 90 + angle;
            } else
            {
                angle = 90 - angle;
            }
            ESP_LOGD(TAG, "iaq tend value=%u, angle=%u", value, angle);
            lv_meter_set_indicator_value(iaq_box_->tend->meter, iaq_box_->tend->needle, angle);
        }
        unlock();
    }
    void updateTimeLabel(uint32_t timestamp, uint32_t timestampOffset)
    {
        lock();

        timeZoneLabel.setCurrentTime(timestamp);
        dateLabel.setCurrentDate(timestamp, "%d.%m.%y");

        // clear previous mark day
        for (int i = 0; i < 7; i++)
            lv_label_set_text(days_header_->current_day[i], "");
        // set current mark day
        lv_label_set_text(days_header_->current_day[getDayOfWeek(timestamp)], CUR_DAY_SYMBOL);

        lv_label_set_text(days_header_->zodizk_sign, getZodiacSign(timestamp).c_str());
        if (tv_)
        {
            if (timestamp != 0)
            {
                time_t   timestampStruct = timestamp;
                struct tm* timeInfo      = localtime(&timestampStruct);

                if (tv_->label)
                {
                    char time_text[6] = { 0 };
                    strftime(time_text, sizeof(time_text), "%H:%M", timeInfo);
                    lv_label_set_text(tv_->label, time_text);
                }

                if (tv_->date_label)
                {
                    char date_text[24] = { 0 };
                    strftime(date_text, sizeof(date_text), "%d %b %Y, %a", timeInfo);
                    lv_label_set_text(tv_->date_label, date_text);
                }
            } else
            {
                if (tv_->label)
                    lv_label_set_text(tv_->label, "");
                if (tv_->date_label)
                    lv_label_set_text(tv_->date_label, "");
            }
        }

        unlock();
    }

    void updatePressureBox(UnitType pressure_unit)
    {
        int32_t scale_min;
        int32_t scale_max;
        switch (pressure_unit)
        {
        case UnitType::hPa:
            scale_min         = 900;
            scale_max         = 1100;
            Y_axis_info_.mode = UnitType::hPa;
            break;
        case UnitType::mmHg:
            scale_min         = 700;
            scale_max         = 800;
            Y_axis_info_.mode = UnitType::mmHg;
            break;
        default:
            scale_min = 90000;
            scale_max = 110000;
            break;
        }

        pressure_box_->gauge->scale->min = scale_min;
        pressure_box_->gauge->scale->max = scale_max;

        lv_chart_set_range(pressure_box_->plot.chart, LV_CHART_AXIS_SECONDARY_Y,
                           (lv_coord_t)scale_min * MULT_COEFF, (lv_coord_t)scale_max * MULT_COEFF);
    }

    static lv_color_t get_quality_color(uint16_t value, std::initializer_list<std::pair<uint16_t, lv_palette_t>> ranges)
    {
        for (const auto& entry : ranges)
        {
            if (value <= entry.first)
                return lv_palette_main(entry.second);
        }
        return lv_palette_main(LV_PALETTE_RED);
    }

    static std::string getZodiacSign(uint32_t timestamp);
    static int         getDayOfWeek(uint32_t timestamp);

    void updateUnitNames();
    void updateSettings(const std::string& old_dev_name);

    int16_t* getPressurePlotRawData()
    {
        _lv_obj_t* chart = pressure_box_->plot.chart;

        lv_chart_series_t* series = lv_chart_get_series_next(chart, nullptr);

        if (series == nullptr)
            return nullptr;
        return lv_chart_get_y_array(chart, series);
    }

    uint16_t getPressurePlotRawDataSize()
    {
        return lv_chart_get_point_count(pressure_box_->plot.chart);
    }

    void updatePressurePlotData(UnitType old_pressure_unit)
    {
        for (auto& dev_name : Aggregator::instance().getPressurePlotDataKeys())
        {
            PlotChartData* plot_data = Aggregator::instance().getPressurePlotData(dev_name);
            if (plot_data == nullptr)
                return;

            int16_t* raw_data;
            if (dev_name == sensor_settings_->sensor_name)
                raw_data = getPressurePlotRawData();
            else
                raw_data = plot_data->data();
            uint16_t data_size = getPressurePlotRawDataSize();

            // Convert data to selected unit type
            float value = 0.0F;
            for (uint16_t i = 0; i < data_size; ++i)
            {
                if (raw_data[i] == LV_CHART_POINT_NONE)
                {
                    value = 0.0F;
                } else
                {
                    value = static_cast<float>(raw_data[i]);
                    value = convertValueToDefault(old_pressure_unit, value);
                    value = convertValueToUnit(sensor_settings_->pressure, value);
                }

                if (dev_name == sensor_settings_->sensor_name)
                    setPressurePlotValue(static_cast<lv_coord_t>(value), i);
                else
                    raw_data[i] = static_cast<int16_t>(value);
            }
        }
    }

    void switchPressurePlotData(const std::string& old_dev_name)
    {
        int16_t* raw_data  = getPressurePlotRawData();
        uint16_t data_size = getPressurePlotRawDataSize();
        Aggregator::instance().savePressurePlotData(old_dev_name, raw_data, data_size);

        PlotChartData* plot_data =
            Aggregator::instance().getPressurePlotData(sensor_settings_->sensor_name);
        if (plot_data == nullptr)
            return;

        plot_data->load();
    }

    void updateSensorData(const std::string& old_dev_name, const std::string& dev_name)
    {
        EnvironmentalSensor::DataSample<float> temperature;
        EnvironmentalSensor::DataSample<float> humidity;
        EnvironmentalSensor::DataSample<float> pressure;
        EnvironmentalSensor::DataSample<float> co2;
        EnvironmentalSensor::DataSample<float> voc;
        EnvironmentalSensor::DataSample<float> iaq;

        Aggregator::instance().getTemperatureData(dev_name, temperature);
        Aggregator::instance().getHumidityData(dev_name, humidity);
        Aggregator::instance().getPressureData(dev_name, pressure);
        Aggregator::instance().getCO2Data(dev_name, co2);
        Aggregator::instance().getVOCData(dev_name, voc);
        Aggregator::instance().getIAQData(dev_name, iaq);

        updateTemperature(dev_name, temperature.value);
        updateHumidity(dev_name, humidity.value);
        updatePressure(dev_name, pressure.value);
        updateCO2(dev_name, co2.value);
        updateVOC(dev_name, voc.value);
        updateIAQ(dev_name, iaq.value);
        updateDewPoint(dev_name);

        switchPressurePlotData(old_dev_name);
        updateSettings(old_dev_name);
    }
};



