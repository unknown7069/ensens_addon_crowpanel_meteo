#include "WeatherForecastParser.h"

#include <json_parser.h>

bool parseWeatherForecast(const char* json, int length, int64_t localNow,
                          int32_t timezoneOffset, WeatherForecast& result)
{
    result = WeatherForecast(localNow);
    if (!json || length <= 0)
        return false;
    jparse_ctx_t parser{};
    if (json_parse_start(&parser, json, length) != OS_SUCCESS)
        return false;

    WeatherForecast parsed(localNow);
    int count = 0;
    bool valid = json_obj_get_array(&parser, "list", &count) == OS_SUCCESS && count > 0;
    for (int i = 0; valid && i < count; ++i)
    {
        int64_t timestamp = 0;
        float temperature = 0;
        char icon[4] = {};
        valid = json_arr_get_object(&parser, i) == OS_SUCCESS &&
                json_obj_get_int64(&parser, "dt", &timestamp) == OS_SUCCESS && timestamp > 0 &&
                json_obj_get_object(&parser, "main") == OS_SUCCESS &&
                json_obj_get_float(&parser, "temp", &temperature) == OS_SUCCESS &&
                json_obj_leave_object(&parser) == OS_SUCCESS;
        if (!valid)
            break;

        // Optional conditions must not discard otherwise useful temperatures.
        // Use a copy of the cursor to avoid unbalanced nesting on bad icons.
        jparse_ctx_t conditions = parser;
        int weatherCount = 0;
        if (json_obj_get_array(&conditions, "weather", &weatherCount) == OS_SUCCESS &&
            weatherCount > 0 && json_arr_get_object(&conditions, 0) == OS_SUCCESS)
            json_obj_get_string(&conditions, "icon", icon, sizeof(icon));

        parsed.add(timestamp + timezoneOffset, temperature, icon);
        valid = json_arr_leave_object(&parser) == OS_SUCCESS;
    }
    json_parse_end(&parser);
    if (!valid || !parsed.count)
        return false;
    result = parsed;
    return true;
}
