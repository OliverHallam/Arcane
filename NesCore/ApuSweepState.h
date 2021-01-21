#pragma once

#include <cstdint>

struct ApuSweepState
{
    uint16_t Period{};
    uint16_t Period2{ 2 };

    bool Enabled{};
    uint_fast8_t Divide{};
    bool Negate{};
    uint_fast8_t Shift{};
    bool Reload{};
    uint_fast8_t DivideCounter{};
    uint16_t TargetPeriod{};
};