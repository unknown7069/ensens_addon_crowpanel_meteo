#pragma once
#include "entities/Aggregator.h"
#include "entities/EnvironmentalSensorData.h"

#include "driver/uart.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <json_parser.h>
#include <string>

#define EX_UART_NUM UART_NUM_1
#define EX_UART_TX GPIO_NUM_20
#define EX_UART_RX GPIO_NUM_19
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
#define TEMP_ADJUST (-2.0F)

class UART
{
    static constexpr char TAG[] = "uart";
    QueueHandle_t         queue = nullptr;
    jparse_ctx_t          jctx  = {};

    static void task(void* pvParameters)
    {
        UART*        uart = static_cast<UART*>(pvParameters);
        uart_event_t event;
        size_t       buffered_size = 0;
        auto dtmp = static_cast<uint8_t*>(heap_caps_calloc(1, RD_BUF_SIZE, MALLOC_CAP_SPIRAM));
        struct EnvironmentalSensor::Data sensor;
        const std::string                dev_name = "indoor";
        for (;;)
        {
            if (!xQueueReceive(uart->queue, &event, portMAX_DELAY))
            {
                continue;
            }
            ESP_LOGD(TAG, "event=%d, timeout=%d, size=%u", event.type, event.timeout_flag,
                     event.size);
            switch (event.type)
            {
            // Event of UART receiving data
            /*We'd better handler data event fast, there would be much more
            data events than other types of events. If we take too much time
            on data event, the queue might be full.*/
            case UART_DATA: {
                uart_read_bytes(EX_UART_NUM, &dtmp[buffered_size], event.size, portMAX_DELAY);
                buffered_size += event.size;
                if (!event.timeout_flag)
                    break;
                ESP_LOGD(TAG, "[UART DATA]: %d, %d", buffered_size, event.timeout_flag ? 1 : 0);
                ESP_LOG_BUFFER_HEXDUMP(TAG, dtmp, buffered_size, ESP_LOG_DEBUG);
                const int ret = json_parse_start(&uart->jctx, reinterpret_cast<char*>(dtmp),
                                                 static_cast<int>(buffered_size));
                buffered_size = 0;
                if (ret != OS_SUCCESS)
                {
                    ESP_LOGE(TAG, "Parser failed: json structure (%d)", ret);
                    break;
                }
                if (json_obj_get_float(&uart->jctx, "temperature", &sensor.temperature) ==
                        OS_SUCCESS &&
                    json_obj_get_float(&uart->jctx, "humidity", &sensor.humidity) == OS_SUCCESS &&
                    json_obj_get_float(&uart->jctx, "pressure", &sensor.pressure) == OS_SUCCESS &&
                    json_obj_get_float(&uart->jctx, "co2", &sensor.co2) == OS_SUCCESS &&
                    json_obj_get_float(&uart->jctx, "voc", &sensor.voc) == OS_SUCCESS &&
                    json_obj_get_float(&uart->jctx, "iaq", &sensor.iaq) == OS_SUCCESS)
                {
                    sensor.temperature += TEMP_ADJUST;
                    EnvironmentalSensor::Flags flags;
                    flags.set_source(EnvironmentalSensor::Source::UART);
                    const time_t time_val = time(nullptr);
                    const auto   t        = static_cast<uint32_t>(time_val);
                    Aggregator::instance().addDevice(dev_name);
                    Aggregator::instance().addTemperatureData(dev_name,
                                                              {
                                                                  .timestamp = t,
                                                                  .flags     = flags,
                                                                  .value     = sensor.temperature,
                                                              });
                    Aggregator::instance().addHumidityData(dev_name, {
                                                                         .timestamp = t,
                                                                         .flags     = flags,
                                                                         .value = sensor.humidity,
                                                                     });
                    Aggregator::instance().addPressureData(
                        dev_name, { .timestamp = t, .flags = flags, .value = sensor.pressure });
                    Aggregator::instance().addCO2Data(dev_name, {
                                                                    .timestamp = t,
                                                                    .flags     = flags,
                                                                    .value     = sensor.co2,
                                                                });
                    Aggregator::instance().addVOCData(dev_name, {
                                                                    .timestamp = t,
                                                                    .flags     = flags,
                                                                    .value     = sensor.voc,
                                                                });
                    Aggregator::instance().addIAQData(dev_name, {
                                                                    .timestamp = t,
                                                                    .flags     = flags,
                                                                    .value     = sensor.iaq,
                                                                });
                    ESP_LOGD(TAG, "Parsed: %.1f, %.1f, %.1f, %.1f, %.1f, %.1f", sensor.temperature,
                             sensor.humidity, sensor.pressure, sensor.co2, sensor.voc, sensor.iaq);
                }
                json_parse_end(&uart->jctx);
                bzero(dtmp, RD_BUF_SIZE);
                break;
            }
            default:
                break;
            }
        }
        free(dtmp);
        dtmp = nullptr;
        vTaskDelete(nullptr);
    }

public:
    static UART& instance()
    {
        static UART instance;
        return instance;
    }

    void init()
    {
        uart_config_t uart_config = {
            .baud_rate  = 115200,
            .data_bits  = UART_DATA_8_BITS,
            .parity     = UART_PARITY_DISABLE,
            .stop_bits  = UART_STOP_BITS_1,
            .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };
        uart_driver_install(EX_UART_NUM, BUF_SIZE * 2, BUF_SIZE * 2, 20, &queue, 0);
        uart_param_config(EX_UART_NUM, &uart_config);
        uart_set_pin(EX_UART_NUM, EX_UART_TX, EX_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
        xTaskCreateWithCaps(task, "uart task", 3072, this, 12, nullptr, MALLOC_CAP_SPIRAM);
    }
};
