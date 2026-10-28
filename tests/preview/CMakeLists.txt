cmake_minimum_required(VERSION 3.16)
project(kitchen_dashboard_preview C CXX)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)
get_filename_component(REPO "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(LVGL_ROOT "${REPO}/managed_components/lvgl__lvgl")
file(GLOB_RECURSE LVGL_SOURCES "${LVGL_ROOT}/src/*.c")
add_library(lvgl_preview STATIC ${LVGL_SOURCES})
target_compile_definitions(lvgl_preview PUBLIC LV_CONF_INCLUDE_SIMPLE LV_KCONFIG_IGNORE)
target_include_directories(lvgl_preview PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${LVGL_ROOT}")

add_executable(dashboard_preview main.cpp
    "${REPO}/sources/entities/ui/Dashboard/DashboardView.cpp"
    "${REPO}/sources/entities/ui/Dashboard/WeatherIcon.cpp"
    "${REPO}/sources/entities/ui/Dashboard/KitchenTheme.cpp"
    "${REPO}/sources/entities/ui/wifi_screen/SettingsView.cpp"
    "${REPO}/sources/entities/ui/fonts/kitchen_clock_188.c"
    "${REPO}/sources/entities/ui/fonts/kitchen_clock_160.c"
    "${REPO}/sources/entities/ui/fonts/kitchen_temperature_88.c"
    "${REPO}/sources/entities/ui/fonts/kitchen_temperature_104.c"
    "${REPO}/sources/entities/ui/fonts/kitchen_temperature_64.c")
target_include_directories(dashboard_preview PRIVATE "${REPO}/sources")
target_link_libraries(dashboard_preview PRIVATE lvgl_preview m)

add_executable(weather_forecast_test "${REPO}/tests/weather_forecast_test.cpp")
target_include_directories(weather_forecast_test PRIVATE "${REPO}/sources")
add_executable(indoor_history_test "${REPO}/tests/indoor_history_test.cpp")
target_include_directories(indoor_history_test PRIVATE "${REPO}/sources")
add_executable(weather_parser_test "${REPO}/tests/weather_parser_test.cpp"
    "${REPO}/sources/entities/WeatherForecastParser.cpp"
    "${REPO}/managed_components/espressif__json_parser/src/json_parser.c")
target_include_directories(weather_parser_test PRIVATE "${REPO}/sources"
    "${REPO}/managed_components/espressif__json_parser/include"
    "${REPO}/managed_components/espressif__jsmn/include")
enable_testing()
add_test(NAME forecast_boundaries COMMAND weather_forecast_test)
add_test(NAME indoor_rolling_history COMMAND indoor_history_test)
add_test(NAME forecast_parser COMMAND weather_parser_test)
add_test(NAME dashboard_layout COMMAND dashboard_preview)
