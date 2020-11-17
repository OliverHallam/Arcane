#pragma once

#include <cstdint>

struct ApuTriangleCoreState
{
    uint32_t Period{};
    uint32_t Period2{ 2 };
    int32_t Timer{};

    int WaveformCycle{};

    uint32_t LinearCounter{};
    bool LinearCounterReload{};
    uint32_t LinearCounterReloadValue{};
    bool Control{};

};