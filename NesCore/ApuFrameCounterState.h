#pragma once

struct ApuFrameCounterState
{
    uint_fast8_t Phase{};
    uint8_t Mode{};
    bool EnableInterrupt{};
};