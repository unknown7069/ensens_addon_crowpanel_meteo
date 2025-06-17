#include "Dashboard.h"

#include "entities/ui/weather_screen/WeatherScreen.h"

lv_obj_t* Dashboard::create(SensorSettings* sensor_settings, lv_obj_t* parent)
{
    lock();
    sensor_settings_ = sensor_settings;

    tv_ = tabview_create(parent, 25);
    create_main_elements(tv_->tab_1);
    WeatherScreen::instance().create(sensor_settings_, tv_);
    WifiScreen::instance().create(sensor_settings_, lv_obj_create(NULL));
    WifiScreen::instance().loadSettings();

    unlock();
    return cont_;
}

void Dashboard::updateUnitNames()
{
    lock();
    const char* pressure_unit_name = unit_names.at(sensor_settings_->pressure);
    lv_label_set_text(pressure_box_->title, pressure_unit_name);
    const char* temperature_unit_name = unit_names.at(sensor_settings_->temperature);
    lv_label_set_text(temp_box_->value->unit_part, temperature_unit_name);
    lv_label_set_text(humi_dew_point_box_->value_dew->unit_part, temperature_unit_name);
    unlock();
}

void Dashboard::updateSettings(const std::string& old_dev_name)
{
    lock();
    updateUnitNames();

    EnvironmentalSensor::DataSample<float> temperature;
    Aggregator::instance().getTemperatureData(sensor_settings_->sensor_name, temperature);

    EnvironmentalSensor::DataSample<float> pressure;
    Aggregator::instance().getPressureData(sensor_settings_->sensor_name, pressure);

    updatePressureBox(sensor_settings_->pressure);
    EnvironmentalSensor::DataSample<float> old_pressure;
    Aggregator::instance().getPressureData(old_dev_name, old_pressure);
    updatePressurePlotData(old_pressure.unit);

    convertToDefault(temperature);
    convertToUnit(sensor_settings_->temperature, temperature);
    updateTemperature(sensor_settings_->sensor_name, temperature.value);

    convertToDefault(pressure);
    convertToUnit(sensor_settings_->pressure, pressure);
    updatePressure(sensor_settings_->sensor_name, pressure.value);
    Aggregator::instance().setPressureData(sensor_settings_->sensor_name, pressure);

    updateDewPoint(sensor_settings_->sensor_name);

    WeatherScreen::instance().updateWeatherValues();

    unlock();
}

std::string Dashboard::getZodiacSign(uint32_t timestamp)
{
    if (timestamp != 0)
    {
        time_t     timestampStruct = timestamp;
        struct tm* timeInfo        = localtime(&timestampStruct);

        int month = timeInfo->tm_mon + 1; // tm_mon is 0-11
        int day   = timeInfo->tm_mday;

        // Determine zodiac sign
        if ((month == 1 && day >= 20) || (month == 2 && day <= 18))
            return "Aquarius";
        if ((month == 2 && day >= 19) || (month == 3 && day <= 20))
            return "Pisces";
        if ((month == 3 && day >= 21) || (month == 4 && day <= 19))
            return "Aries";
        if ((month == 4 && day >= 20) || (month == 5 && day <= 20))
            return "Taurus";
        if ((month == 5 && day >= 21) || (month == 6 && day <= 20))
            return "Gemini";
        if ((month == 6 && day >= 21) || (month == 7 && day <= 22))
            return "Cancer";
        if ((month == 7 && day >= 23) || (month == 8 && day <= 22))
            return "Leo";
        if ((month == 8 && day >= 23) || (month == 9 && day <= 22))
            return "Virgo";
        if ((month == 9 && day >= 23) || (month == 10 && day <= 22))
            return "Libra";
        if ((month == 10 && day >= 23) || (month == 11 && day <= 21))
            return "Scorpio";
        if ((month == 11 && day >= 22) || (month == 12 && day <= 21))
            return "Sagittarius";
        if ((month == 12 && day >= 22) || (month == 1 && day <= 19))
            return "Capricorn";
        return "Unknown"; // Should never reach here
    } else
        return "";

    return "Unknown";
}

int Dashboard::getDayOfWeek(uint32_t timestamp)
{
    if (timestamp != 0)
    {
        time_t     timestampStruct = timestamp;
        struct tm* timeInfo        = localtime(&timestampStruct);

        return timeInfo->tm_wday; // days since Sunday - [0, 6]
    } else
        return 0;

    return 0;
}