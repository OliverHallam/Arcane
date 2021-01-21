#pragma once

#include <cstdint>

struct ApuNoiseCoreState
{
    uint_fast16_t ModeShift{1};
    uint_fast16_t Period{};
    uint32_t Period2{2};

    int32_t Timer{};
    uint_fast16_t Shifter{1};
};