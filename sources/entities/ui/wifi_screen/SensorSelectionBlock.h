#pragma once
#include "adapters/lvgl/Button.h"
#include "adapters/lvgl/ExpandableBlock.h"
#include "adapters/lvgl/FlexContainer.h"
#include "adapters/lvgl/ScreenBase.h"
#include "adapters/lvgl/SimpleLabel.h"
#include "adapters/lvgl/MenuPage.h"
#include "entities/ui/sensors_settings/SensorSettings.h"

class SensorSelectionBlock : public LvglObject
{
    static constexpr char* Tag = "SensorSelection";
    FlexContainer          spacer;

    static constexpr uint8_t MaxSensorNameLength = 20;
    std::vector<std::string> names               = {};
    SensorSettings*          settings            = nullptr;
    char                     saved_sens_name[MaxSensorNameLength];

    MenuPage      page;
    FlexContainer sensorEntryContainer;
    SimpleLabel   sensorEntryLabel;
    lv_obj_t*     sensorDropDown = nullptr;
    void          sensorDropDownHandler(lv_event_t* e);
    bool          saveSensorName(const char* sens_name);
    void          updateSensorDropDown(const uint16_t pos);

public:
    void create(Menu&, SensorSettings*);
    void updateNames(const std::vector<std::string>&);
    bool getSensorName();
};