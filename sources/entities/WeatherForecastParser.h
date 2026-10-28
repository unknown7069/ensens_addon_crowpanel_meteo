#pragma once

#include "WeatherForecast.h"

// Parse into a temporary result, so malformed responses never publish half a
// forecast. The weather icon is optional; temperature and timestamp are not.
bool parseWeatherForecast(const char* json, int length, int64_t localNow,
                          int32_t timezoneOffset, WeatherForecast& result);
