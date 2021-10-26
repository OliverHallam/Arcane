#include "MMC6.h"

#include "..\CartCoreState.h"
#include "..\CartData.h"

void MMC6::Initialize(CartCoreState& state, CartData& data)
{
    MMC3::Initialize(state, data);

    state.ReadMap[3] = Read0x6xxx;
    state.WriteMap[4] = Write0x8xxx;
    state.WriteMap[5] = Write0xaxxx;

    auto& mmc6State = state.MapperState.MMC6;

    mmc6State.PrgRamProtect0 = 0;
    mmc6State.PrgRamProtect1 = 0;
}

void MMC6::Write0x8xxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    MMC3::Write0x8xxx(bus, state, data, address, value);

    if ((address & 1) == 0)
    {
        auto mmc6State = state.MapperState.MMC6;

        // PRG RAM enable
        // use bit 3 of the protect flag to signal this 
        if ((value & 0x20) != 0)
        {
            mmc6State.PrgRamProtect0 |= 4;
            mmc6State.PrgRamProtect1 |= 4;
        }
        else
        {
            mmc6State.PrgRamProtect0 &= ~4;
            mmc6State.PrgRamProtect1 &= ~4;
        }
    }
}

void MMC6::UpdateChrMap(CartCoreState& state, CartData& data)
{
}

void MMC6::Write0xaxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    if ((address & 1) == 0)
    {
        MMC3::Write0xaxxx(bus, state, data, address, value);
    }
    else
    {
        auto& mmc6State = state.MapperState.MMC6;

        mmc6State.PrgRamProtect0 &= ~3;
        mmc6State.PrgRamProtect0 |= (value >> 4 & 3);

        mmc6State.PrgRamProtect1 &= ~3;
        mmc6State.PrgRamProtect1 |= (value >> 6 & 3);
    }
}

uint8_t MMC6::Read0x6xxx(CartCoreState& state, CartData& data, uint16_t address)
{
    if (address >= 0x7000)
    {
        auto& mmc6State = state.MapperState.MMC6;
        auto protect = (address & 0x200) != 0 ? mmc6State.PrgRamProtect1 : mmc6State.PrgRamProtect0;
        if ((protect & 6) != 0)
            return data.PrgRamBanks[0][address & 0x3ff];
    }

    return 0;
}