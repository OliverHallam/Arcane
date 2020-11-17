#pragma once

#include <cstdint>

struct ApuLengthCounterState
{
    bool Enabled{};
    bool Halt{};
    uint8_t Length{};
};