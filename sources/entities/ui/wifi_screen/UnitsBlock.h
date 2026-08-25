#pragma once

#include "adapters/lvgl/Button.h"
#include "adapters/lvgl/ExpandableBlock.h"
#include "adapters/lvgl/FlexContainer.h"
#include "adapters/lvgl/ScreenBase.h"
#include "adapters/lvgl/SimpleLabel.h"
#include "adapters/lvgl/MenuPage.h"
#include "entities/ui/sensors_settings/SensorSettings.h"

class UnitsBlock
{
    static constexpr const char* TAG = "UnitsBlock";
    MenuPage               page;
    FlexContainer          spacer;
    FlexContainer          temperatureUnitsEntryContainer;
    SimpleLabel            temperatureUnitsEntryLabel;
    lv_obj_t*              temperatureDropDown = nullptr;
    FlexContainer          pressureUnitsEntryContainer;
    SimpleLabel            pressureUnitsEntryLabel;
    lv_obj_t*              pressureDropDown = nullptr;

    void unitsConfigurationHandler(lv_event_t*);
    SensorSettings* sensor_settings_;
    void            updateUnitsDropdown();
    void            saveSettings();

public:
    void create(Menu&, SensorSettings*);
    void loadSettings();
};
