#pragma once
#include <cstdlib>
#define MALLOC_CAP_SPIRAM 0
inline void* heap_caps_calloc(size_t count, size_t size, int) { return std::calloc(count, size); }
