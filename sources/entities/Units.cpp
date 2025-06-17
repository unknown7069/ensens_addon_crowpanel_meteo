#include "Units.h"

#include "EnvironmentalSensorData.h"

// Note:
// These template specializations are defined in the .cpp file to avoid a
// circular dependency:
// - EnvironmentalSensor::DataSample depends on Units.h (for UnitType)
// - convertToUnit (in Units.h) needs to operate on DataSample
//
// To break this cycle, the template is declared in Units.h and specialized
// here.
//
// This limitation could be resolved by extracting DataSample into a separate
// header that doesn't depend on Units.h. That would allow the specialization to
// be moved into Units.h directly, removing the need for templates entirely.

template <> void convertToDefault(EnvironmentalSensor::DataSample<float>& data)
{
    switch (data.unit)
    {
    case hPa:
        data.value = data.value * 100;
        data.unit  = Pa;
        break;
    case mmHg:
        data.value = data.value / 0.00750062f;
        data.unit  = Pa;
        break;
    case Fahrenheit:
        data.value = (data.value - 32.f) / 1.8f;
        data.unit  = Celsius;
        break;
    default:
        break;
    }
}

template <> void convertToUnit(const UnitType to_unit, EnvironmentalSensor::DataSample<float>& data)
{
    switch (to_unit)
    {
    case hPa:
        data.value = data.value * 0.01f;
        data.unit  = hPa;
        break;
    case mmHg:
        data.value = data.value * 0.00750062f;
        data.unit  = mmHg;
        break;
    case Fahrenheit:
        data.value = data.value * 1.8f + 32.f;
        data.unit  = Fahrenheit;
        break;
    default:
        break;
    }
}

float convertValueToDefault(const UnitType from_unit, float value)
{
    switch (from_unit)
    {
    case hPa:
        return value * 100;
    case mmHg:
        return value / 0.00750062f;
    case Fahrenheit:
        return (value - 32.f) / 1.8f;
    default:
        return value;
    }
}

float convertValueToUnit(const UnitType to_unit, float value)
{
    switch (to_unit)
    {
    case hPa:
        return value * 0.01f;
    case mmHg:
        return value * 0.00750062f;
    case Fahrenheit:
        return value * 1.8f + 32.f;
    default:
        return value;
    }
}