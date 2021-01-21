#pragma once

#include <cstdint>

struct PpuBackgroundState
{
    uint16_t CurrentAddress{};
    uint8_t FineX{};
    uint16_t BackgroundPatternBase{};
    uint32_t LeftCrop{};
};