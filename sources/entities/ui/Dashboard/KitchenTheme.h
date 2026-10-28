#pragma once

#include "lvgl.h"

namespace KitchenTheme {
constexpr uint32_t Paper = 0xF2EDE3;
constexpr uint32_t Oatmeal = 0xE7E0D3;
constexpr uint32_t Ink = 0x292927;
constexpr uint32_t Muted = 0x625F59;
constexpr uint32_t Rule = 0xCBC3B5;
constexpr uint32_t Terracotta = 0xB96C40;
constexpr uint32_t Slate = 0x64758A;

// Shared by the clock, sensor details, all settings pages and popup controls.
void initialize(lv_disp_t* display);
void surface(lv_obj_t* object, uint32_t color = Paper, int radius = 0);
}
