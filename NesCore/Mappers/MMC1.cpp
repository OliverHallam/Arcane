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

    mmc1state.BankSelect = 0;
    mmc1state.CommandNumber = 0;
    mmc1state.MapperShift = 0;
    mmc1state.MapperShiftCount = 0;
    mmc1state.PrgMode = 3;
}

void MMC1::Write(CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
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
        WriteRegister(state, data, address, mmc1state.MapperShift);

        mmc1state.MapperShiftCount = 0;
        mmc1state.MapperShift = 0;
    }
}

void MMC1::Write2(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
    // The MMC1 takes the first value and ignores the second.
    Write(state, data, address, firstValue);
    bus.TickCpuWrite();
}

void MMC1::A12Rising(CartCoreState& state, CartData& data)
{
    if (state.ChrA12Sensitivity == ChrA12Sensitivity::AllEdges)
    {
        UpdatePrgMap(state, data);
    }
}

void MMC1::A12Falling(CartCoreState& state, CartData& data)
{
    if (state.ChrA12Sensitivity == ChrA12Sensitivity::AllEdges)
    {
        UpdatePrgMap(state, data);
    }
}

void MMC1::WriteRegister(CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto& mmc1state = state.MapperState.MMC1;

    switch (address >> 13)
    {
    case 4:
        // control

        // chr mode
        mmc1state.ChrMode = (value >> 4) & 0x01;
        UpdateChrMap(state, data);

        // prg mode
        mmc1state.PrgMode = (value >> 2) & 0x03;
        UpdatePrgMap(state, data);

        // mirroring mode
        mmc1state.MirrorMode = static_cast<MirrorMode>(value & 0x03);
        UpdatePpuRamMap(bus, state);
        break;

    case 5:
    {
        // CHR bank 0
        mmc1state.ChrBank0 = value << 12;
        UpdateChrMapMMC1();

        auto sensitivityBefore = state.ChrA12Sensitivity;

        state.ChrA12Sensitivity = ChrA12Sensitivity::None;

        // TODO: SNROM PRG RAM disable line
        if (prgData_.size() == 0x80000)
        {
            mmc1state.PrgPlane0 = ((mmc1state.ChrBank0 << 2) & 0x40000);
            if (mmc1state.PrgPlane0 != mmc1state.PrgPlane1)
                state_.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (!state_.ChrA12)
                UpdatePrgMapMMC1();
        }

        if (prgRamBanks_.size() > 1)
        {
            if (prgRamBanks_.size() > 2)
                mmc1state.PrgRamBank0 = (value >> 2) & 0x03;
            else
                mmc1state.PrgRamBank0 = (value >> 1) & 0x01;

            if (mmc1state.PrgRamBank0 != mmc1state.PrgRamBank1)
                state_.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (!state_.ChrA12 && state_.CpuBanks[3])
                state_.CpuBanks[3] = prgRamBanks_[state_.PrgRamBank0];
        }

        if (state_.ChrA12Sensitivity != sensitivityBefore)
            bus_->UpdateA12Sensitivity();
        break;
    }

    case 6:
    {
        // CHR bank 1
        mmc1state.ChrBank1 = value << 12;
        UpdateChrMapMMC1();

        auto sensitivityBefore = state_.ChrA12Sensitivity;
        state_.ChrA12Sensitivity = ChrA12Sensitivity::None;

        // TODO: SNROM PRG RAM disable line
        if (prgData_.size() == 0x80000)
        {
            mmc1state.PrgPlane1 = ((mmc1state.ChrBank1 << 2) & 0x40000);

            if (mmc1state.PrgPlane0 != mmc1state.PrgPlane1)
                state_.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (state_.ChrA12)
                UpdatePrgMapMMC1();
        }

        // TODO: SZROM maps this differently.
        if (prgRamBanks_.size() > 1)
        {
            if (prgRamBanks_.size() > 2)
                mmc1state.PrgRamBank1 = (value >> 2) & 0x03;
            else
                mmc1state.PrgRamBank1 = (value >> 1) & 0x01;

            if (mmc1state.PrgRamBank0 != mmc1state.PrgRamBank1)
                state_.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (state_.ChrA12 && state_.CpuBanks[3])
                state_.CpuBanks[3] = prgRamBanks_[state_.PrgRamBank1];
        }

        if (state_.ChrA12Sensitivity != sensitivityBefore)
            bus_->UpdateA12Sensitivity();

        break;
    }

    case 7:
        // PRG bank

        // enable/disable PRG RAM
        mmc1state.PrgRamEnabled = (value & 0x10) == 0;
        mmc1state.PrgBank = ((value & 0x0f) << 14 & prgMask_);
        UpdatePrgMapMMC1();
        break;
    }
}

void MMC1::UpdateChrMap(CartCoreState state)
{
    bus_->SyncPpu();

    switch (state_.ChrMode)
    {
    case 0:
    {
        auto chrBank = static_cast<size_t>(state_.ChrBank0) & 0x1e000;
        auto base = &chrData_[chrBank];
        state_.PpuBanks[0] = base;
        state_.PpuBanks[1] = base + 0x0400;
        state_.PpuBanks[2] = base + 0x0800;
        state_.PpuBanks[3] = base + 0x0c00;
        state_.PpuBanks[4] = base + 0x1000;
        state_.PpuBanks[5] = base + 0x1400;
        state_.PpuBanks[6] = base + 0x1800;
        state_.PpuBanks[7] = base + 0x1c00;
        break;
    }

    case 1:
    {
        auto base0 = &chrData_[state_.ChrBank0 & (chrData_.size() - 1)];
        state_.PpuBanks[0] = base0;
        state_.PpuBanks[1] = base0 + 0x0400;
        state_.PpuBanks[2] = base0 + 0x0800;
        state_.PpuBanks[3] = base0 + 0x0c00;

        auto base1 = &chrData_[state_.ChrBank1 & (chrData_.size() - 1)];
        state_.PpuBanks[4] = base1;
        state_.PpuBanks[5] = base1 + 0x0400;
        state_.PpuBanks[6] = base1 + 0x0800;
        state_.PpuBanks[7] = base1 + 0x0c00;
        break;
    }
    }
}

void MMC1::UpdatePrgMap()
{
    if (!state_.PrgRamEnabled || prgRamBanks_.size() == 0)
        state_.CpuBanks[3] = nullptr;
    else
        state_.CpuBanks[3] = state_.ChrA12 ? prgRamBanks_[state_.PrgRamBank1] : prgRamBanks_[state_.PrgRamBank0];

    auto prgPlane = state_.ChrA12 ? state_.PrgPlane1 : state_.PrgPlane0;

    switch (state_.PrgMode)
    {
    case 0:
    case 1:
    {
        auto base = &prgData_[prgPlane | (state_.PrgBank0 & 0xffff8000)];
        state_.CpuBanks[4] = base;
        state_.CpuBanks[5] = base + 0x2000;
        state_.CpuBanks[6] = base + 0x4000;
        state_.CpuBanks[7] = base + 0x6000;
        break;
    }

    case 2:
    {
        auto base = &prgData_[prgPlane | state_.PrgBank0];
        state_.CpuBanks[4] = &prgData_[prgPlane];
        state_.CpuBanks[5] = &prgData_[prgPlane | 0x2000];
        state_.CpuBanks[6] = base;
        state_.CpuBanks[7] = base + 0x2000;
        break;
    }

    case 3:
    {
        auto base = &prgData_[prgPlane | state_.PrgBank0];
        state_.CpuBanks[4] = base;
        state_.CpuBanks[5] = base + 0x2000;
        state_.CpuBanks[6] = &prgData_[prgPlane | (prgData_.size() - 0x4000)];
        state_.CpuBanks[7] = &prgData_[prgPlane | (prgData_.size() - 0x2000)];
    }
    }
}