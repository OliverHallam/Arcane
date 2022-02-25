#include "MMC1.h"

#include "MapperUtils.h"

#include "../Bus.h"
#include "../CartCoreState.h"

void MMC1::Initialize(CartCoreState& state)
{
    state.WriteMap[4] = Write;
    state.WriteMap[5] = Write;
    state.WriteMap[6] = Write;
    state.WriteMap[7] = Write;

    state.Write2Map[4] = Write2;
    state.Write2Map[5] = Write2;
    state.Write2Map[6] = Write2;
    state.Write2Map[7] = Write2;

    auto& mmc1state = state.MapperState.MMC1;

    mmc1state.MapperShiftCount = 0;
    mmc1state.MapperShift = 0;
    mmc1state.BankSelect = 0;
    mmc1state.CommandNumber = 0;
    mmc1state.MirrorMode = MirrorMode::Horizontal;
    mmc1state.ChrMode = 0;
    mmc1state.ChrBank0 = 0;
    mmc1state.ChrBank1 = 0;
    mmc1state.PrgMode = 3;
    mmc1state.PrgPlane0 = 0;
    mmc1state.PrgPlane1 = 0;
    mmc1state.PrgBank = 0;
    mmc1state.PrgRamEnabled = 0;
    mmc1state.PrgRamBank0 = 0;
    mmc1state.PrgRamBank1 = 0;
    mmc1state.ChrA12 = false;
}

void MMC1::Write(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto& mmc1state = state.MapperState.MMC1;
    if ((value & 0x80) == 0x80)
    {
        mmc1state.MapperShiftCount = 0;
        mmc1state.MapperShift = 0;

        // when the shift register is reset, the control bit is or'd with 0x0C
        mmc1state.PrgMode = 3;
        UpdatePrgMap(state, data);
        return;
    }

    mmc1state.MapperShift |= (value & 1) << mmc1state.MapperShiftCount;
    mmc1state.MapperShiftCount++;

    if (mmc1state.MapperShiftCount == 5)
    {
        WriteRegister(bus, state, data, address, mmc1state.MapperShift);

        mmc1state.MapperShiftCount = 0;
        mmc1state.MapperShift = 0;
    }
}

void MMC1::Write2(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
    // The MMC1 takes the first value and ignores the second.
    Write(bus, state, data, address, firstValue);
    bus.TickCpuWrite();
}

void MMC1::A12Rising(CartCoreState& state, CartData& data)
{
    auto& mmc1state = state.MapperState.MMC1;
    mmc1state.ChrA12 = true;

    if (state.ChrA12Sensitivity == ChrA12Sensitivity::AllEdges)
    {
        UpdatePrgMap(state, data);
    }
}

void MMC1::A12Falling(CartCoreState& state, CartData& data)
{
    auto& mmc1state = state.MapperState.MMC1;
    mmc1state.ChrA12 = false;

    if (state.ChrA12Sensitivity == ChrA12Sensitivity::AllEdges)
    {
        UpdatePrgMap(state, data);
    }
}

void MMC1::WriteRegister(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto& mmc1State = state.MapperState.MMC1;

    switch (address >> 13)
    {
    case 4:
        // control

        // chr mode
        mmc1State.ChrMode = (value >> 4) & 0x01;
        UpdateChrMap(bus, state, data);

        // prg mode
        mmc1State.PrgMode = (value >> 2) & 0x03;
        UpdatePrgMap(state, data);

        // mirroring mode
        mmc1State.MirrorMode = static_cast<MirrorMode>(value & 0x03);
        UpdatePpuRamMap(bus, state);
        break;

    case 5:
    {
        // CHR bank 0
        mmc1State.ChrBank0 = value << 12;
        UpdateChrMap(bus, state, data);

        auto sensitivityBefore = state.ChrA12Sensitivity;

        state.ChrA12Sensitivity = ChrA12Sensitivity::None;

        // TODO: SNROM PRG RAM disable line
        if (data.PrgData.size() == 0x80000)
        {
            mmc1State.PrgPlane0 = ((mmc1State.ChrBank0 << 2) & 0x40000);
            if (mmc1State.PrgPlane0 != mmc1State.PrgPlane1)
                state.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (!mmc1State.ChrA12)
                UpdatePrgMap(state, data);
        }

        if (data.PrgRamBanks.size() > 1)
        {
            if (data.PrgRamBanks.size() > 2)
                mmc1State.PrgRamBank0 = (value >> 2) & 0x03;
            else
                mmc1State.PrgRamBank0 = (value >> 1) & 0x01;

            if (mmc1State.PrgRamBank0 != mmc1State.PrgRamBank1)
                state.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (!mmc1State.ChrA12 && state.CpuBanks[3])
                state.CpuBanks[3] = data.PrgRamBanks[mmc1State.PrgRamBank0];
        }

        if (state.ChrA12Sensitivity != sensitivityBefore)
            bus.UpdateA12Sensitivity();
        break;
    }

    case 6:
    {
        // CHR bank 1
        mmc1State.ChrBank1 = value << 12;
        UpdateChrMap(bus, state, data);

        auto sensitivityBefore = state.ChrA12Sensitivity;
        state.ChrA12Sensitivity = ChrA12Sensitivity::None;

        // TODO: SNROM PRG RAM disable line
        if (data.PrgData.size() == 0x80000)
        {
            mmc1State.PrgPlane1 = ((mmc1State.ChrBank1 << 2) & 0x40000);

            if (mmc1State.PrgPlane0 != mmc1State.PrgPlane1)
                state.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (mmc1State.ChrA12)
                UpdatePrgMap(state, data);
        }

        // TODO: SZROM maps this differently.
        if (data.PrgRamBanks.size() > 1)
        {
            if (data.PrgRamBanks.size() > 2)
                mmc1State.PrgRamBank1 = (value >> 2) & 0x03;
            else
                mmc1State.PrgRamBank1 = (value >> 1) & 0x01;

            if (mmc1State.PrgRamBank0 != mmc1State.PrgRamBank1)
                state.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (mmc1State.ChrA12 && state.CpuBanks[3])
                state.CpuBanks[3] = data.PrgRamBanks[mmc1State.PrgRamBank1];
        }

        if (state.ChrA12Sensitivity != sensitivityBefore)
            bus.UpdateA12Sensitivity();

        break;
    }

    case 7:
        // PRG bank

        // enable/disable PRG RAM
        mmc1State.PrgRamEnabled = (value & 0x10) == 0;
        mmc1State.PrgBank = ((value & 0x0f) << 14 & data.PrgMask);
        UpdatePrgMap(state, data);
        break;
    }
}

void MMC1::UpdateChrMap(Bus& bus, CartCoreState& state, CartData& data)
{
    auto mmc1state = state.MapperState.MMC1;

    bus.SyncPpu();

    switch (mmc1state.ChrMode)
    {
    case 0:
    {
        auto chrBank = static_cast<size_t>(mmc1state.ChrBank0) & 0x1e000;
        auto base = &data.ChrData[chrBank];
        state.PpuBanks[0] = base;
        state.PpuBanks[1] = base + 0x0400;
        state.PpuBanks[2] = base + 0x0800;
        state.PpuBanks[3] = base + 0x0c00;
        state.PpuBanks[4] = base + 0x1000;
        state.PpuBanks[5] = base + 0x1400;
        state.PpuBanks[6] = base + 0x1800;
        state.PpuBanks[7] = base + 0x1c00;
        break;
    }

    case 1:
    {
        auto base0 = &data.ChrData[mmc1state.ChrBank0 & (data.ChrData.size() - 1)];
        state.PpuBanks[0] = base0;
        state.PpuBanks[1] = base0 + 0x0400;
        state.PpuBanks[2] = base0 + 0x0800;
        state.PpuBanks[3] = base0 + 0x0c00;

        auto base1 = &data.ChrData[mmc1state.ChrBank1 & (data.ChrData.size() - 1)];
        state.PpuBanks[4] = base1;
        state.PpuBanks[5] = base1 + 0x0400;
        state.PpuBanks[6] = base1 + 0x0800;
        state.PpuBanks[7] = base1 + 0x0c00;
        break;
    }
    }
}

void MMC1::UpdatePrgMap(CartCoreState& state, CartData& data)
{
    auto& mmc1State = state.MapperState.MMC1;

    if (!mmc1State.PrgRamEnabled || data.PrgRamBanks.size() == 0)
        state.CpuBanks[3] = nullptr;
    else
        state.CpuBanks[3] = mmc1State.ChrA12 ? data.PrgRamBanks[mmc1State.PrgRamBank1] : data.PrgRamBanks[mmc1State.PrgRamBank0];

    auto prgPlane = mmc1State.ChrA12 ? mmc1State.PrgPlane1 : mmc1State.PrgPlane0;

    switch (mmc1State.PrgMode)
    {
    case 0:
    case 1:
    {
        auto base = &data.PrgData[prgPlane | (mmc1State.PrgBank & 0xffff8000)];
        state.CpuBanks[4] = base;
        state.CpuBanks[5] = base + 0x2000;
        state.CpuBanks[6] = base + 0x4000;
        state.CpuBanks[7] = base + 0x6000;
        break;
    }

    case 2:
    {
        auto base = &data.PrgData[prgPlane | mmc1State.PrgBank];
        state.CpuBanks[4] = &data.PrgData[prgPlane];
        state.CpuBanks[5] = &data.PrgData[prgPlane | 0x2000];
        state.CpuBanks[6] = base;
        state.CpuBanks[7] = base + 0x2000;
        break;
    }

    case 3:
    {
        auto base = &data.PrgData[prgPlane | mmc1State.PrgBank];
        state.CpuBanks[4] = base;
        state.CpuBanks[5] = base + 0x2000;
        state.CpuBanks[6] = &data.PrgData[prgPlane | (data.PrgData.size() - 0x4000)];
        state.CpuBanks[7] = &data.PrgData[prgPlane | (data.PrgData.size() - 0x2000)];
    }
    }
}