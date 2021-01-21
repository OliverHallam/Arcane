#pragma once

#include <memory>
#include <cstdint>

struct ApuCoreState
{
    uint32_t CurrentSample{};
    uint32_t SampleCycle{};

    uint32_t LastSyncCycle{};

    bool DmcInterrupt{};
    bool FrameCounterInterrupt{};
};