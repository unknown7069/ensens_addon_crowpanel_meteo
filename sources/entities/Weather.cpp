#include "Weather.h"
#include "adapters/HTTPRequest.h"
#include "../certs/OpenWeatherCACert.h"
#include "esp_heap_caps.h"
#include <esp_log.h>
#include <json_parser.h>
#include <ctime>
#include <limits>

char* urlEncode(const char* input)
{
    size_t len     = strlen(input);
    char*  encoded = static_cast<char*>(malloc(len * 3 + 1));
    if (!encoded)
        return nullptr;

    char* p = encoded;
    while (*input)
    {
        if (isalnum((unsigned char)*input) || *input == '-' || *input == '_' || *input == '.' ||
            *input == '~')
        {
            *p++ = *input;
        } else
        {
            sprintf(p, "%%%02X", (unsigned char)*input);
            p += 3;
        }
        input++;
    }
    *p = '\0';
    return encoded;
}

Weather::Weather()
{
    ctx_ = static_cast<Ctx*>(heap_caps_malloc(sizeof(Ctx), MALLOC_CAP_SPIRAM));
}

bool Weather::getCurrentWeather(Data* data)
{
    if (!data)
        return false;

    if ((ctx_->latitude[0] != '\0') && (ctx_->longitude[0] != '\0'))
        snprintf(ctx_->requestURL, sizeof(ctx_->requestURL), "%slat=%s&lon=%s&appid=%s",
                 WEATHER_URL, ctx_->latitude, ctx_->longitude, WEATHER_API_KEY);
    else if (ctx_->locationName[0] != '0')
    {
        char* encodedCity = urlEncode(ctx_->locationName);
        if (!encodedCity)
            return false;

        snprintf(ctx_->requestURL, sizeof(ctx_->requestURL), "%sq=%s&appid=%s", WEATHER_URL,
                 encodedCity, WEATHER_API_KEY);
        free(encodedCity);
    } else
    {
        ESP_LOGE(Tag, "Location not ready");
        return false;
    }

    HTTPRequest request(ctx_->requestURL, HTTP_METHOD_GET, ctx_->openWeatherDataBuffer,
                        sizeof(ctx_->openWeatherDataBuffer), OpenWeatherRootCACert);

    jparse_ctx_t jctx;
    int          numCnt = 0;
    uint32_t     receivedLen = 0;
    bool         retVal = true;
    int64_t      timestamp;
    int64_t      timezone;
    int64_t      sunrise;
    int64_t      sunset;
    if (((receivedLen = request.perform()) > 0) &&
        (json_parse_start(&jctx, ctx_->openWeatherDataBuffer, receivedLen) == OS_SUCCESS) &&
        (json_obj_get_array(&jctx, "weather", &numCnt) == OS_SUCCESS) &&
        (json_arr_get_object(&jctx, 0) == OS_SUCCESS) &&
        (json_obj_get_string(&jctx, "description", data->description, 64) == OS_SUCCESS) &&
        (json_obj_get_string(&jctx, "icon", data->icon, 64) == OS_SUCCESS) &&
        (json_arr_leave_object(&jctx) == OS_SUCCESS) &&
        (json_obj_leave_array(&jctx) == OS_SUCCESS) &&
        (json_obj_get_object(&jctx, "main") == OS_SUCCESS) &&
        (json_obj_get_float(&jctx, "temp", &data->temperature) == OS_SUCCESS) &&
        (json_obj_get_float(&jctx, "feels_like", &data->feelsLike) == OS_SUCCESS) &&
        (json_obj_get_float(&jctx, "pressure", &data->pressure) == OS_SUCCESS) &&
        (json_obj_get_float(&jctx, "humidity", &data->humidity) == OS_SUCCESS) &&
        (json_obj_get_float(&jctx, "temp_min", &data->tempMin) == OS_SUCCESS) &&
        (json_obj_get_float(&jctx, "temp_max", &data->tempMax) == OS_SUCCESS) &&
        (json_obj_leave_object(&jctx) == OS_SUCCESS) &&
        (json_obj_get_object(&jctx, "wind") == OS_SUCCESS) &&
        (json_obj_get_float(&jctx, "speed", &data->windSpeed) == OS_SUCCESS) &&
        (json_obj_get_float(&jctx, "deg", &data->windDir) == OS_SUCCESS) &&
        (json_obj_leave_object(&jctx) == OS_SUCCESS) &&
        (json_obj_get_int64(&jctx, "dt", &timestamp) == OS_SUCCESS) &&
        (json_obj_get_object(&jctx, "sys") == OS_SUCCESS) &&
        (json_obj_get_string(&jctx, "country", data->country, sizeof(data->country)) ==
         OS_SUCCESS) &&
        (json_obj_get_int64(&jctx, "sunrise", &sunrise) == OS_SUCCESS) &&
        (json_obj_get_int64(&jctx, "sunset", &sunset) == OS_SUCCESS) &&
        (json_obj_leave_object(&jctx) == OS_SUCCESS) &&
        (json_obj_get_int64(&jctx, "timezone", &timezone) == OS_SUCCESS) &&
        (json_obj_get_string(&jctx, "name", data->city, sizeof(data->city)) == OS_SUCCESS))
    {
        ESP_LOGD(Tag, "Parsed: %.1f, %.1f, %.1f, %s,", data->temperature, data->humidity,
                 data->pressure, data->description);
        auto clampToUint32 = [](int64_t value) -> uint32_t {
            if (value < 0)
                return 0;
            if (value > std::numeric_limits<uint32_t>::max())
                return std::numeric_limits<uint32_t>::max();
            return static_cast<uint32_t>(value);
        };

        data->timestamp        = clampToUint32(timestamp + timezone);
        data->timestampOffset  = static_cast<int32_t>(timezone);
        data->sunriseTimestamp = clampToUint32(sunrise + timezone);
        data->sunsetTimestamp  = clampToUint32(sunset + timezone);
        ESP_LOGI(Tag, "%lu, %ld, %s, %s", data->timestamp,
                 static_cast<long>(data->timestampOffset), data->city,
                 data->country);
    } else
    {
        ESP_LOGE(Tag, "Parser failed: objects");
        retVal = false;
    }
    json_parse_end(&jctx);

    if (retVal && (receivedLen > 0) &&
        ((ctx_->latitude[0] == '\0') || (ctx_->longitude[0] == '\0')))
    {
        float        coordLat  = 0.0f;
        float        coordLon  = 0.0f;
        jparse_ctx_t coordCtx  = { 0 };
        if ((json_parse_start(&coordCtx, ctx_->openWeatherDataBuffer, receivedLen) == OS_SUCCESS) &&
            (json_obj_get_object(&coordCtx, "coord") == OS_SUCCESS) &&
            (json_obj_get_float(&coordCtx, "lon", &coordLon) == OS_SUCCESS) &&
            (json_obj_get_float(&coordCtx, "lat", &coordLat) == OS_SUCCESS) &&
            (json_obj_leave_object(&coordCtx) == OS_SUCCESS))
        {
            snprintf(ctx_->latitude, sizeof(ctx_->latitude), "%.4f", coordLat);
            snprintf(ctx_->longitude, sizeof(ctx_->longitude), "%.4f", coordLon);
        }
        json_parse_end(&coordCtx);
    }

    memset(ctx_->openWeatherDataBuffer, 0, sizeof(ctx_->openWeatherDataBuffer));
    data->temperature -= 273.15f;
    data->feelsLike -= 273.15f;
    data->tempMin -= 273.15f;
    data->tempMax -= 273.15f;
    return retVal;
}

bool Weather::getForecast(Data* data)
{
    if (!data)
        return false;

    static constexpr uint8_t ForecastEntryCount  = 40;
    static constexpr uint8_t HourlyForecastCount = 4;
    static constexpr uint8_t DailyForecastCount  = 4;
    static constexpr uint8_t DailyStartIndex     = HourlyForecastCount;
    memset(data, 0, sizeof(Data) * ForecastEntryCount);

    if ((ctx_->latitude[0] != '\0') && (ctx_->longitude[0] != '\0'))
    {
        snprintf(ctx_->requestURL, sizeof(ctx_->requestURL), "%slat=%s&lon=%s&appid=%s",
                 FORECAST_URL, ctx_->latitude, ctx_->longitude, WEATHER_API_KEY);
    } else if (ctx_->locationName[0] != '\0')
    {
        char* encodedCity = urlEncode(ctx_->locationName);
        if (!encodedCity)
            return false;

        snprintf(ctx_->requestURL, sizeof(ctx_->requestURL), "%sq=%s&appid=%s", FORECAST_URL,
                 encodedCity, WEATHER_API_KEY);
        free(encodedCity);
    } else
    {
        ESP_LOGE(Tag, "Location not ready");
        return false;
    }

    HTTPRequest request(ctx_->requestURL, HTTP_METHOD_GET, ctx_->openWeatherDataBuffer,
                        sizeof(ctx_->openWeatherDataBuffer), OpenWeatherRootCACert);

    jparse_ctx_t jctx;
    int          listCount   = 0;
    int          weatherCnt  = 0;
    uint32_t     receivedLen = 0;
    bool         retVal      = true;
    int64_t      timestamp   = 0;
    int64_t      timezone    = 0;

    if (((receivedLen = request.perform()) > 0) &&
        (json_parse_start(&jctx, ctx_->openWeatherDataBuffer, receivedLen) == OS_SUCCESS) &&
        (json_obj_get_array(&jctx, "list", &listCount) == OS_SUCCESS))
    {
        const int maxEntries = (listCount < ForecastEntryCount) ? listCount : ForecastEntryCount;
        for (int i = 0; (i < maxEntries) && retVal; i++)
        {
            if ((json_arr_get_object(&jctx, i) == OS_SUCCESS) &&
                (json_obj_get_int64(&jctx, "dt", &timestamp) == OS_SUCCESS) &&
                (json_obj_get_object(&jctx, "main") == OS_SUCCESS) &&
                (json_obj_get_float(&jctx, "temp", &data[i].temperature) == OS_SUCCESS) &&
                (json_obj_get_float(&jctx, "temp_min", &data[i].tempMin) == OS_SUCCESS) &&
                (json_obj_get_float(&jctx, "temp_max", &data[i].tempMax) == OS_SUCCESS) &&
                (json_obj_leave_object(&jctx) == OS_SUCCESS) &&
                (json_obj_get_array(&jctx, "weather", &weatherCnt) == OS_SUCCESS) &&
                (json_arr_get_object(&jctx, 0) == OS_SUCCESS) &&
                (json_obj_get_string(&jctx, "icon", data[i].icon, sizeof(data[i].icon)) ==
                 OS_SUCCESS) &&
                (json_arr_leave_object(&jctx) == OS_SUCCESS) &&
                (json_obj_leave_array(&jctx) == OS_SUCCESS) &&
                (json_arr_leave_object(&jctx) == OS_SUCCESS))
            {
                data[i].timestamp = static_cast<uint32_t>(timestamp);
            } else
            {
                ESP_LOGE(Tag, "Parser failed: forecast list entry");
                retVal = false;
            }
        }
    } else
    {
        ESP_LOGE(Tag, "Parser failed: forecast list");
        retVal = false;
    }

    if (retVal &&
        (json_obj_leave_array(&jctx) == OS_SUCCESS) &&
        (json_obj_get_object(&jctx, "city") == OS_SUCCESS) &&
        (json_obj_get_int64(&jctx, "timezone", &timezone) == OS_SUCCESS))
    {
        float coordLat = 0.0f;
        float coordLon = 0.0f;
        if ((json_obj_get_object(&jctx, "coord") == OS_SUCCESS) &&
            (json_obj_get_float(&jctx, "lat", &coordLat) == OS_SUCCESS) &&
            (json_obj_get_float(&jctx, "lon", &coordLon) == OS_SUCCESS) &&
            (json_obj_leave_object(&jctx) == OS_SUCCESS))
        {
            snprintf(ctx_->latitude, sizeof(ctx_->latitude), "%.4f", coordLat);
            snprintf(ctx_->longitude, sizeof(ctx_->longitude), "%.4f", coordLon);
        }
        retVal &= (json_obj_leave_object(&jctx) == OS_SUCCESS);
    } else
    {
        ESP_LOGE(Tag, "Parser failed: city metadata");
        retVal = false;
    }

    json_parse_end(&jctx);

    if (!retVal)
    {
        memset(ctx_->openWeatherDataBuffer, 0, sizeof(ctx_->openWeatherDataBuffer));
        return false;
    }

    for (int i = 0; i < ForecastEntryCount; i++)
    {
        if (data[i].timestamp == 0)
            break;
        int64_t adjustedTs = static_cast<int64_t>(data[i].timestamp) + timezone;
        if (adjustedTs < 0)
            adjustedTs = 0;
        data[i].timestamp = static_cast<uint32_t>(adjustedTs);
        data[i].temperature -= 273.15f;
        data[i].tempMin -= 273.15f;
        data[i].tempMax -= 273.15f;
    }

    struct DailyAggregate {
        int      year      = -1;
        int      yday      = -1;
        float    minTemp   = 0.0f;
        float    maxTemp   = 0.0f;
        uint32_t timestamp = 0;
        int      hourDelta = 24;
        char     icon[sizeof(data[0].icon)] = {};
        bool     assigned  = false;
    };

    DailyAggregate daily[DailyForecastCount];
    int            dailyCount = 0;

    time_t   utcNow    = time(nullptr);
    time_t   localNow  = utcNow + timezone;
    struct tm localNowTm;
    gmtime_r(&localNow, &localNowTm);
    int referenceYear = localNowTm.tm_year;
    int referenceYDay = localNowTm.tm_yday;

    for (int i = 0; i < ForecastEntryCount; i++)
    {
        if (data[i].timestamp == 0)
            break;

        time_t    t   = data[i].timestamp;
        struct tm tmv;
        gmtime_r(&t, &tmv);

        if (tmv.tm_year < referenceYear ||
            (tmv.tm_year == referenceYear && tmv.tm_yday <= referenceYDay))
            continue;

        int target = -1;
        for (int j = 0; j < dailyCount; j++)
        {
            if (daily[j].yday == tmv.tm_yday && daily[j].year == tmv.tm_year)
            {
                target = j;
                break;
            }
        }

        if (target == -1)
        {
            if (dailyCount >= DailyForecastCount)
                continue;
            target                  = dailyCount++;
            daily[target].year      = tmv.tm_year;
            daily[target].yday      = tmv.tm_yday;
            daily[target].minTemp   = data[i].tempMin;
            daily[target].maxTemp   = data[i].tempMax;
            daily[target].timestamp = data[i].timestamp;
            daily[target].hourDelta =
                (tmv.tm_hour > 12) ? (tmv.tm_hour - 12) : (12 - tmv.tm_hour);
            strncpy(daily[target].icon, data[i].icon, sizeof(daily[target].icon));
            daily[target].assigned = true;
            continue;
        }

        if (!daily[target].assigned || data[i].tempMin < daily[target].minTemp)
            daily[target].minTemp = data[i].tempMin;
        if (data[i].tempMax > daily[target].maxTemp)
            daily[target].maxTemp = data[i].tempMax;

        int currentDelta = (tmv.tm_hour > 12) ? (tmv.tm_hour - 12) : (12 - tmv.tm_hour);
        if (currentDelta < daily[target].hourDelta)
        {
            daily[target].hourDelta = currentDelta;
            daily[target].timestamp = data[i].timestamp;
            strncpy(daily[target].icon, data[i].icon, sizeof(daily[target].icon));
        }
    }

    for (int i = 0; i < dailyCount; i++)
    {
        uint8_t idx        = DailyStartIndex + static_cast<uint8_t>(i);
        data[idx].timestamp = daily[i].timestamp;
        data[idx].tempMin   = daily[i].minTemp;
        data[idx].tempMax   = daily[i].maxTemp;
        data[idx].temperature = (daily[i].minTemp + daily[i].maxTemp) * 0.5f;
        strncpy(data[idx].icon, daily[i].icon, sizeof(daily[i].icon));
    }

    memset(ctx_->openWeatherDataBuffer, 0, sizeof(ctx_->openWeatherDataBuffer));
    return (data[0].timestamp != 0) && (data[DailyStartIndex].timestamp != 0);
}

bool Weather::getNextDayIdx(Data* data, uint8_t* curIdx)
{
    if (!data || !curIdx || (*curIdx >= 40))
        return false;
    struct tm tm_first;
    gmtime_r((time_t*)&data[*curIdx].timestamp, &tm_first);

    for (; *curIdx < 40; (*curIdx)++)
    {
        struct tm tm_current;
        gmtime_r((time_t*)&data[*curIdx].timestamp, &tm_current);

        if (tm_current.tm_year != tm_first.tm_year || tm_current.tm_mon != tm_first.tm_mon ||
            tm_current.tm_mday != tm_first.tm_mday)
        {
            return true;
        }
    }
    return false;
}

void Weather::setLocation(char* lat, char* lon, char* name)
{
    if (!lat || !lon)
        return;
    strncpy(ctx_->latitude, lat, sizeof(ctx_->latitude));
    strncpy(ctx_->longitude, lon, sizeof(ctx_->longitude));
    strncpy(ctx_->locationName, name, sizeof(ctx_->locationName));
}

bool Weather::checkLocation(char* name)
{
    if (!name)
        return false;

    if (name[0] == '\0')
        return true;

    static constexpr uint16_t ResponseBufferSize = 1024;
    char*                     encodedCity        = urlEncode(name);
    if (!encodedCity)
        return false;

    char* requestBuffer =
        static_cast<char*>(heap_caps_malloc(RequestBufferSize, MALLOC_CAP_SPIRAM));
    if (!requestBuffer)
    {
        ESP_LOGE(Tag, "Malloc request failed");
        free(encodedCity);
        return false;
    }
    char* responseBuffer =
        static_cast<char*>(heap_caps_malloc(ResponseBufferSize, MALLOC_CAP_SPIRAM));
    if (!responseBuffer)
    {
        free(encodedCity);
        free(requestBuffer);
        ESP_LOGE(Tag, "Malloc response failed");
        return false;
    }

    snprintf(requestBuffer, RequestBufferSize, "%sq=%s&appid=%s", WEATHER_URL, encodedCity,
             WEATHER_API_KEY);
    free(encodedCity);

    HTTPRequest request(requestBuffer, HTTP_METHOD_POST, responseBuffer, ResponseBufferSize,
                        OpenWeatherRootCACert);

    jparse_ctx_t jctx;
    uint32_t     receivedLen;
    bool         retVal = true;
    int          code   = 0;
    if (((receivedLen = request.perform()) > 0) &&
        (json_parse_start(&jctx, responseBuffer, receivedLen) == OS_SUCCESS) &&
        (json_obj_get_int(&jctx, "cod", &code) == OS_SUCCESS) && (code == 200))
        ESP_LOGI(Tag, "Location is correct, code - %d", code);
    else
    {
        ESP_LOGI(Tag, "Location not found, code - %d", code);
        retVal = false;
    }
    json_parse_end(&jctx);
    free(requestBuffer);
    free(responseBuffer);
    return retVal;
}

