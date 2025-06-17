#include "SensorSelectionBlock.h"
#include "entities/ui/Dashboard/Dashboard.h"

void SensorSelectionBlock::create(Menu& menu_, SensorSettings* sensorSettings)
{
    settings = sensorSettings;
}

void SensorSelectionBlock::updateNames(const std::vector<std::string>& sensorNames)
{
    lock();
    names = sensorNames;

    settings->sensor_name = saved_sens_name;
    unlock();
}

void SensorSelectionBlock::sensorDropDownHandler(lv_event_t* e)
{
    lock();

    if (names.empty())
        return;
    const uint16_t sel          = lv_dropdown_get_selected(lv_event_get_target(e));
    std::string    old_dev_name = settings->sensor_name;
    settings->sensor_name       = names[sel];

    saveSensorName(names[sel].c_str());
    Dashboard::instance().updateSensorData(old_dev_name, settings->sensor_name);

    unlock();
}

bool SensorSelectionBlock::saveSensorName(const char* sens_name)
{
    nvs_handle_t handle;

    esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(Tag, "Failed to open NVS: %s", esp_err_to_name(err));
        return false;
    }

    if (!sens_name || strlen(sens_name) == 0)
    {
        err = nvs_erase_key(handle, "sensor_name");
        if (err == ESP_OK)
        {
            ESP_LOGI(Tag, "Sensor name removed from NVS");
            nvs_commit(handle);
            nvs_close(handle);
            return true;
        } else if (err == ESP_ERR_NVS_NOT_FOUND)
        {
            ESP_LOGW(Tag, "Sensor name already cleared");
            nvs_close(handle);
            return true;
        } else
        {
            ESP_LOGE(Tag, "Failed to erase sensor_name: %s", esp_err_to_name(err));
            nvs_close(handle);
            return false;
        }
    }

    err = nvs_set_str(handle, "sensor_name", sens_name);
    if (err != ESP_OK)
    {
        ESP_LOGE(Tag, "Failed to set sensor_name: %s", esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }

    err = nvs_commit(handle);
    nvs_close(handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(Tag, "Failed to commit sensor_name: %s", esp_err_to_name(err));
        return false;
    }

    ESP_LOGD(Tag, "Sensor name saved: %s", sens_name);
    return true;
}

bool SensorSelectionBlock::getSensorName()
{
    nvs_handle_t handle;
    size_t       size   = MaxSensorNameLength;
    bool         retVal = true;

    strcpy(saved_sens_name, "indoor\0");

    ESP_LOGD(Tag, "Sensor name opened: %s", saved_sens_name);
    return retVal;
}

void SensorSelectionBlock::updateSensorDropDown(const uint16_t pos)
{
    lv_dropdown_set_selected(sensorDropDown, pos);
    lv_dropdown_set_selected_highlight(sensorDropDown, true);
}