#pragma once

#define TENDENCY_UPDATE_PERIOD_MS (5 * 60 * 1000)                   // 5 min
#define HISTORY_UPDATE_PERIOD_MS (15 * 60 * 1000)                   // 15 min
#define HISTORY_LENGTH_MS (60 * 60 * 1000 * 24)                     // 1 day
#define HISTORY_SIZE (HISTORY_LENGTH_MS / HISTORY_UPDATE_PERIOD_MS) // 96: every 15 mins
constexpr size_t HALF_HISTORY_SIZE = HISTORY_SIZE / 2;

#define DEVICE_ADV_UPDATE_PERIOD_MS 3000
#define DEVICE_CHAR_UPDATE_PERIOD_MS (4 * 60 * 1000) // 4 min
#define DEVICE_CONNECT_TIMEOUT_MS 2000
#define DEVICE_NUM 7
