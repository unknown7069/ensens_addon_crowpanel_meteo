#ifndef SENSORSETTINGS_H
#define SENSORSETTINGS_H

#include <string>

#include "entities/Units.h"

struct SensorSettings {
    UnitType    pressure    = hPa;
    std::string sensor_name = "indoor";
    UnitType    temperature = Celsius;
};

#endif // SENSORSETTINGS_H
