#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// RAII guard that takes a FreeRTOS mutex on construction and gives it back on
// destruction.
class MutexGuard
{
public:
    explicit MutexGuard(SemaphoreHandle_t mutex) : mutex_(mutex)
    {
        xSemaphoreTake(mutex_, portMAX_DELAY);
    }
    ~MutexGuard()
    {
        xSemaphoreGive(mutex_);
    }

    MutexGuard(const MutexGuard&)            = delete;
    MutexGuard& operator=(const MutexGuard&) = delete;

private:
    SemaphoreHandle_t mutex_;
};
