#include "MapperUtils.h"
#include "..\Bus.h"
#include "..\CartCoreState.h"

void UpdatePpuRamMap(Bus& bus, CartCoreState& state)
{
    auto base = bus.GetPpuRamBase();
    switch (state.MirrorMode)
    {
    case MirrorMode::SingleScreenLow:
        state.PpuBanks[8] = state.PpuBanks[12] = base;
        state.PpuBanks[9] = state.PpuBanks[13] = base;
        state.PpuBanks[10] = state.PpuBanks[14] = base;
        state.PpuBanks[11] = state.PpuBanks[15] = base;
        break;

    case MirrorMode::SingleScreenHigh:
        state.PpuBanks[8] = state.PpuBanks[12] = base + 0x400;
        state.PpuBanks[9] = state.PpuBanks[13] = base + 0x400;
        state.PpuBanks[10] = state.PpuBanks[14] = base + 0x400;
        state.PpuBanks[11] = state.PpuBanks[15] = base + 0x400;
        break;

    case MirrorMode::Vertical:
        state.PpuBanks[8] = state.PpuBanks[12] = base;
        state.PpuBanks[9] = state.PpuBanks[13] = base + 0x400;
        state.PpuBanks[10] = state.PpuBanks[14] = base;
        state.PpuBanks[11] = state.PpuBanks[15] = base + 0x400;
        break;

    case MirrorMode::Horizontal:
        state.PpuBanks[8] = state.PpuBanks[12] = base;
        state.PpuBanks[9] = state.PpuBanks[13] = base;
        state.PpuBanks[10] = state.PpuBanks[14] = base + 0x400;
        state.PpuBanks[11] = state.PpuBanks[15] = base + 0x400;
        break;
    }
}