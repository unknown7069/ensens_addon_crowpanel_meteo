#include "Weather.h"
#include "adapters/HTTPRequest.h"
#include "../certs/OpenWeatherCACert.h"
#include "esp_heap_caps.h"
#include <esp_log.h>
#include <json_parser.h>
#include <limits>
#include <cstring>

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
    if (ctx_ == nullptr)
    {
        ESP_LOGE(TAG, "Failed to allocate weather context");
        return;
    }
    memset(ctx_, 0, sizeof(Ctx));
}

bool Weather::getCurrentWeather(Data* data)
{
    if (!data || !ctx_)
        return false;

    data->precipitation = std::numeric_limits<float>::quiet_NaN();

    if ((ctx_->latitude[0] != '\0') && (ctx_->longitude[0] != '\0'))
        snprintf(ctx_->requestURL, sizeof(ctx_->requestURL), "%slat=%s&lon=%s&appid=%s",
                 WEATHER_URL, ctx_->latitude, ctx_->longitude, WEATHER_API_KEY);
    else if (ctx_->locationName[0] != '\0')
    {
        char* encodedCity = urlEncode(ctx_->locationName);
        if (!encodedCity)
            return false;

        snprintf(ctx_->requestURL, sizeof(ctx_->requestURL), "%sq=%s&appid=%s", WEATHER_URL,
                 encodedCity, WEATHER_API_KEY);
        free(encodedCity);
    } else
    {
        ESP_LOGE(TAG, "Location not ready");
        return false;
    }

    HTTPRequest request(ctx_->requestURL, HTTP_METHOD_GET, ctx_->openWeatherDataBuffer,
                        sizeof(ctx_->openWeatherDataBuffer), OpenWeatherRootCACert);

    jparse_ctx_t jctx;
    int          numCnt = 0;
    int32_t      receivedLen = 0;
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
        ESP_LOGD(TAG, "Parsed: %.1f, %.1f, %.1f, %s,", data->temperature, data->humidity,
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
        ESP_LOGI(TAG, "%lu, %ld, %s, %s", data->timestamp,
                 static_cast<long>(data->timestampOffset), data->city,
                 data->country);

        float precipitation = 0.0f;
        bool  precipFound   = false;
        if (json_obj_get_object(&jctx, "rain") == OS_SUCCESS)
        {
            float rainVal = 0.0f;
            if ((json_obj_get_float(&jctx, "1h", &rainVal) == OS_SUCCESS) ||
                (json_obj_get_float(&jctx, "3h", &rainVal) == OS_SUCCESS))
            {
                precipitation += rainVal;
                precipFound = true;
            }
            json_obj_leave_object(&jctx);
        }
        if (json_obj_get_object(&jctx, "snow") == OS_SUCCESS)
        {
            float snowVal = 0.0f;
            if ((json_obj_get_float(&jctx, "1h", &snowVal) == OS_SUCCESS) ||
                (json_obj_get_float(&jctx, "3h", &snowVal) == OS_SUCCESS))
            {
                precipitation += snowVal;
                precipFound = true;
            }
            json_obj_leave_object(&jctx);
        }

        data->precipitation = precipFound ? precipitation : 0.0f;
    } else
    {
        ESP_LOGE(TAG, "Parser failed: objects");
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


void Weather::setLocation(char* lat, char* lon, char* name)
{
    if (!ctx_ || !lat || !lon || !name)
        return;
    strncpy(ctx_->latitude, lat, sizeof(ctx_->latitude) - 1);
    ctx_->latitude[sizeof(ctx_->latitude) - 1] = '\0';
    strncpy(ctx_->longitude, lon, sizeof(ctx_->longitude) - 1);
    ctx_->longitude[sizeof(ctx_->longitude) - 1] = '\0';
    strncpy(ctx_->locationName, name, sizeof(ctx_->locationName) - 1);
    ctx_->locationName[sizeof(ctx_->locationName) - 1] = '\0';
}

bool Weather::checkLocation(char* name)
{
    if (!name || !ctx_)
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
        ESP_LOGE(TAG, "Malloc request failed");
        free(encodedCity);
        return false;
    }
    char* responseBuffer =
        static_cast<char*>(heap_caps_malloc(ResponseBufferSize, MALLOC_CAP_SPIRAM));
    if (!responseBuffer)
    {
        free(encodedCity);
        free(requestBuffer);
        ESP_LOGE(TAG, "Malloc response failed");
        return false;
    }

    snprintf(requestBuffer, RequestBufferSize, "%sq=%s&appid=%s", WEATHER_URL, encodedCity,
             WEATHER_API_KEY);
    free(encodedCity);

    HTTPRequest request(requestBuffer, HTTP_METHOD_POST, responseBuffer, ResponseBufferSize,
                        OpenWeatherRootCACert);

    jparse_ctx_t jctx;
    int32_t      receivedLen;
    bool         retVal = true;
    int          code   = 0;
    if (((receivedLen = request.perform()) > 0) &&
        (json_parse_start(&jctx, responseBuffer, receivedLen) == OS_SUCCESS) &&
        (json_obj_get_int(&jctx, "cod", &code) == OS_SUCCESS) && (code == 200))
        ESP_LOGI(TAG, "Location is correct, code - %d", code);
    else
    {
        ESP_LOGI(TAG, "Location not found, code - %d", code);
        retVal = false;
    }
    json_parse_end(&jctx);
    free(requestBuffer);
    free(responseBuffer);
    return retVal;
}

