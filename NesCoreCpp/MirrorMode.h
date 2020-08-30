#pragma once

#include <cstdint>

enum class MirrorMode : uint8_t
{
    SingleScreenLow = 0,
    SingleScreenHigh = 1,
    Vertical = 2,
    Horizontal = 3,
    FourScreen = 4
};