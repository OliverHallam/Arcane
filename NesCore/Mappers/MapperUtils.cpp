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

void UpdateChrMap8k(CartCoreState& state, CartData& data, uint32_t bank)
{
    auto base = &data.ChrData[(bank << 13) & data.ChrMask];
    state.PpuBanks[0] = base;
    state.PpuBanks[1] = base + 0x0400;
    state.PpuBanks[2] = base + 0x0800;
    state.PpuBanks[3] = base + 0x0c00;
    state.PpuBanks[4] = base + 0x1000;
    state.PpuBanks[5] = base + 0x1400;
    state.PpuBanks[6] = base + 0x1800;
    state.PpuBanks[7] = base + 0x1c00;
}
