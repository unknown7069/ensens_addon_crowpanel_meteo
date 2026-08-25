#pragma once

#include <stdint.h>

#define WEATHER_API_KEY "b4d1299a4d4bb3e2ed262199a1c61cae"
//#define WEATHER_URL
//"https://api.openweathermap.org/data/2.5/weather?q=Kazan&appid="
// WEATHER_API_KEY
#define WEATHER_URL "https://api.openweathermap.org/data/2.5/weather?"

class Weather
{
    static constexpr uint16_t RequestBufferSize = 256;
    static constexpr char     TAG[]             = "weather";
    struct Ctx {
        char openWeatherDataBuffer[32768];
        char longitude[20];
        char latitude[20];
        char requestURL[RequestBufferSize];
        char locationName[150];
    } * ctx_;

    Weather();
    Weather(const Weather&)            = delete;
    Weather& operator=(const Weather&) = delete;

public:
    static Weather& instance()
    {
        static Weather instance;
        return instance;
    }

    struct Data {
        float    temperature;
        float    humidity;
        float    feelsLike;
        float    tempMin;
        float    tempMax;
        char     description[64];
        char     city[64];
        char     country[64];
        uint32_t timestamp;
        int32_t  timestampOffset;
        char     icon[64];
        float    windSpeed;
        float    windDir;
        float    pressure;
        uint32_t sunriseTimestamp;
        uint32_t sunsetTimestamp;
        float    precipitation;
    };

    bool getCurrentWeather(Data* data);

    void setLocation(char* lat, char* lon, char* name);

    bool checkLocation(char* name);
};
