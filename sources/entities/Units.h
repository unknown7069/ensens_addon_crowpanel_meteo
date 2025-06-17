#pragma once

#include <cstdint>
#include <unordered_map>

enum UnitType
{
    Celsius,
    Fahrenheit,
    Pa,
    hPa,
    mmHg
};

const uint8_t temperature_unit_types_count = 2;

const std::unordered_map<UnitType, const char*> unit_names = { { Celsius, "°C" },
                                                               { Fahrenheit, "°F" },
                                                               { Pa, "Pa" },
                                                               { hPa, "hPa" },
                                                               { mmHg, "mmHg" } };

// Converts "data.unit" to Celsius or Pa; changes "data.unit"
template <typename T> void convertToDefault(T& data);

// Assuming "data.unit" is Celsius or Pa, convert it to "to_unit"; changes "data.unit"
template <typename T> void convertToUnit(UnitType to_unit, T& data);

// Converts "from_unit" to Celsius or Pa
float convertValueToDefault(UnitType from_unit, float value);

// Assuming "value" is in Celsius or Pa, convert it to "to_unit"
float convertValueToUnit(UnitType to_unit, float value);
