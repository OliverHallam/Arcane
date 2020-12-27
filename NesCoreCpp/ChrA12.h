#pragma once

#include "SignalEdge.h"

#include <cstdint>

class PpuBackground;
class PpuSprites;

// Simulates the 12th bit of the PPU address bus, on a render scanline.
class ChrA12
{
public:
    ChrA12(const PpuBackground& background, const PpuSprites& sprites);

    int32_t NextEdgeCycle(int32_t cycle);
    int32_t NextRaisingEdgeCycleFiltered(int32_t cycle, bool isLow);
    int32_t NextTrailingEdgeCycle(int32_t cycle);

    SignalEdge GetEdge(int32_t& cycle, bool smoothed);

private:
    const PpuBackground& background_;
    const PpuSprites& sprites_;
};