#include "MMC3.h"

#include "MapperUtils.h"

#include "../Bus.h"
#include "../CartCoreState.h"

void MMC3::Initialize(CartCoreState& state, CartData& data)
{
    state.WriteMap[4] = Write0x8xxx;
    state.WriteMap[5] = Write0xaxxx;
    state.WriteMap[6] = Write0xcxxx;
    state.WriteMap[7] = Write0xexxx;

    state.Write2Map[4] = Write2;
    state.Write2Map[5] = Write2;
    state.Write2Map[6] = Write2;
    state.Write2Map[7] = Write2;

    auto& mmc3state = state.MapperState.MMC3;
    
    mmc3state.BankSelect = 0;
    mmc3state.PrgMode = 0;
    mmc3state.PrgBank0 = 0;
    mmc3state.PrgBank1 = 0;
    // RAMBO-1 extends MMC3 to support a third PRG register - we fix this for the other variants.
    mmc3state.PrgBank2 = data.PrgBlockSize - 0x4000;

    mmc3state.ChrMode = 0;
    mmc3state.IrqEnabled = false;
    mmc3state.IrqCounter = 0;
    mmc3state.ReloadCounter = false;
    mmc3state.ReloadValue = 0;

}

void MMC3::Write0x8xxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto& mmc3State = state.MapperState.MMC3;

    if ((address & 1) == 0)
    {
        bus.SyncPpu();
        mmc3State.BankSelect = value & 0x07;
        mmc3State.PrgMode = (value >> 6) & 1;
        mmc3State.ChrMode = (value >> 7) & 1;
        UpdatePrgMap(state, data);
        UpdateChrMap(state, data);
    }
    else
    {
        SetBank(bus, state, data, value);
    }
}

void MMC3::Write0xaxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    if ((address & 1) == 0)
    {
        auto mirrorMode = value & 1 ? MirrorMode::Horizontal : MirrorMode::Vertical;
        if (state.MirrorMode != mirrorMode)
        {
            bus.SyncPpu();
            state.MirrorMode = mirrorMode;
            UpdatePpuRamMap(bus, state);
        }
    }
    else
    {
        auto& mmc3State = state.MapperState.MMC3;

        mmc3State.PrgRamProtect0 &= ~3;
        mmc3State.PrgRamProtect0 |= ((value >> 6) & 3);
        UpdatePrgMap(state, data);
    }
}

void MMC3::Write0xcxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto& mmc3State = state.MapperState.MMC3;

    if ((address & 1) == 0)
    {
        mmc3State.ReloadValue = value;
    }
    else
    {
        mmc3State.ReloadCounter = true;

#ifdef DIAGNOSTIC
        bus_->MarkDiagnostic(0xff444444);
#endif
    }

    UpdateChrA12Sensitivity(bus, state, ChrA12Sensitivity::RisingEdgeSmoothed);
}

void MMC3::Write0xexxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto mmc3State = state.MapperState.MMC3;

    if ((address & 1) == 0)
    {
        mmc3State.IrqEnabled = false;
        bus.SetCartIrq(false);
    }
    else
    {
        mmc3State.IrqEnabled = true;
    }

    UpdateChrA12Sensitivity(bus, state, ChrA12Sensitivity::RisingEdgeSmoothed);
}

void MMC3::UpdatePrgMap(CartCoreState& state, CartData& data)
{
    if (state_.PrgRamEnabled && data_.PrgRamBanks.size())
        state_.CpuBanks[3] = data_.PrgRamBanks[0];
    else
        state_.CpuBanks[3] = nullptr;

    auto block = &data_.PrgData[state_.PrgBankHighBits & data_.PrgMask];

    if (state_.PrgMode == 0)
    {
        state_.CpuBanks[4] = &block[state_.PrgBank0 & data_.PrgMask];
        state_.CpuBanks[5] = &block[state_.PrgBank1 & data_.PrgMask];
        state_.CpuBanks[6] = &block[state_.PrgBank2 & data_.PrgMask];
        state_.CpuBanks[7] = &block[data_.PrgBlockSize - 0x2000];
    }
    else
    {
        state_.CpuBanks[4] = &block[state_.PrgBank2 & data_.PrgMask];
        state_.CpuBanks[5] = &block[state_.PrgBank1 & data_.PrgMask];
        state_.CpuBanks[6] = &block[state_.PrgBank0 & data_.PrgMask];
        state_.CpuBanks[7] = &block[data_.PrgBlockSize - 0x2000];
    }
}

void MMC3::SetBank(Bus& bus, CartCoreState& state, CartData& data, uint32_t bank)
{
    auto& mmc3State = state.MapperState.MMC3;

    switch (state.BankSelect)
    {
    case 0:
        bus.SyncPpu();

        mmc3State.ChrBank0 = bank << 10;
        UpdateChrMap(state, data);
        break;

    case 1:
        bus.SyncPpu();
        mmc3State.ChrBank1 = bank << 10;
        UpdateChrMap(state, data);
        break;

    case 2:
        bus.SyncPpu();
        mmc3State.ChrBank2 = bank << 10;
        UpdateChrMap(state, data);
        break;

    case 3:
        bus.SyncPpu();
        mmc3State.ChrBank3 = bank << 10;
        UpdateChrMap(state, data);
        break;

    case 4:
        bus.SyncPpu();
        mmc3State.ChrBank4 = bank << 10;
        UpdateChrMap(state, data);
        break;

    case 5:
        bus.SyncPpu();
        mmc3State.ChrBank5 = bank << 10;
        UpdateChrMap(state, data);
        break;

    case 6:
        mmc3State.PrgBank0 = bank << 13;
        UpdatePrgMap(state, data);
        break;

    case 7:
        mmc3State.PrgBank1 = bank << 13;
        UpdatePrgMap(state, data);
        break;

        // RAMBO-1 extensions:
    case 8:
        bus.SyncPpu();
        mmc3State.ChrBank6 = bank << 10;
        UpdateChrMap(state, data);
        break;

    case 9:
        bus.SyncPpu();
        mmc3State.ChrBank7 = bank << 10;
        UpdateChrMap(state, data);
        break;

    case 15:
        mmc3State.PrgBank2 = bank << 13;
        UpdatePrgMap(state, data);
        break;
    }
}

void MMC3::ClockIrqCounter(Bus& bus, CartCoreState& state)
{
    auto& mmc3State = state.MapperState.MMC3;

    if (mmc3State.IrqCounter == 0 || mmc3State.ReloadCounter)
    {
        mmc3State.IrqCounter = state_.ReloadValue;

        // RAMBO-1 often skips a scanline before resetting - we'll emulate this by resetting earlier, but bumping the count.
        if (mmc3State.BumpIrqCounter)
            mmc3State.IrqCounter += 1;

        mmc3State.BumpIrqCounter = false;
        mmc3State.ReloadCounter = false;

        if ((mapper_ != MapperType::Rambo1 && mapper_ != MapperType::Tengen800037) && mmc3State.IrqCounter == 0 && !mmc3State.IrqEnabled)
        {
            mmc3State.ChrA12Sensitivity = ChrA12Sensitivity::None;
            bus.UpdateA12Sensitivity();
        }
    }
    else
    {
        mmc3State.IrqCounter--;
    }

    if (mmc3State.IrqCounter == 0 && mmc3State.IrqEnabled)
    {
        if (mapper_ == MapperType::Rambo1 || mapper_ == MapperType::Tengen800037)
        {
            bus.Schedule(3, SyncEvent::CartSetIrq);
        }
        else
        {
            bus.SetCartIrq(true);
        }
    }
}

void MMC3::UpdateChrMap(CartCoreState& state, CartData& data)
{
    if (mapper_ == MapperType::TQROM)
    {
        UpdateChrMapTQROM();
        return;
    }

    auto block = &data_.ChrData[state_.ChrBankHighBits & data_.ChrMask];

    if ((state_.ChrMode & 1) == 0)
    {
        if ((state_.ChrMode & 2) != 0)
        {
            // RAMBO-1 full 1kb mode
            state_.PpuBanks[0] = &block[state_.ChrBank0 & data_.ChrMask];
            state_.PpuBanks[1] = &block[state_.ChrBank6 & data_.ChrMask];
            state_.PpuBanks[2] = &block[state_.ChrBank1 & data_.ChrMask];
            state_.PpuBanks[3] = &block[state_.ChrBank7 & data_.ChrMask];
        }
        else
        {
            auto base0 = &block[state_.ChrBank0 & data_.ChrMask & 0xfffff800];
            state_.PpuBanks[0] = base0;
            state_.PpuBanks[1] = base0 + 0x400;

            auto base1 = &block[state_.ChrBank1 & data_.ChrMask & 0xfffff800];
            state_.PpuBanks[2] = base1;
            state_.PpuBanks[3] = base1 + 0x400;
        }

        state_.PpuBanks[4] = &block[state_.ChrBank2 & data_.ChrMask];
        state_.PpuBanks[5] = &block[state_.ChrBank3 & data_.ChrMask];
        state_.PpuBanks[6] = &block[state_.ChrBank4 & data_.ChrMask];
        state_.PpuBanks[7] = &block[state_.ChrBank5 & data_.ChrMask];


        if (mapper_ == MapperType::TxSROM || mapper_ == MapperType::Tengen800037)
        {
            auto base = bus.GetPpuRamBase();
            state_.PpuBanks[8] = state_.PpuBanks[12] = &base[(state_.ChrBank0 >> 7) & 0x00400];
            state_.PpuBanks[9] = state_.PpuBanks[13] = &base[(state_.ChrBank0 >> 7) & 0x00400];
            state_.PpuBanks[10] = state_.PpuBanks[14] = &base[(state_.ChrBank1 >> 7) & 0x00400];
            state_.PpuBanks[11] = state_.PpuBanks[15] = &base[(state_.ChrBank1 >> 7) & 0x00400];
        }
    }
    else
    {
        state_.PpuBanks[0] = &block[state_.ChrBank2 & data_.ChrMask];
        state_.PpuBanks[1] = &block[state_.ChrBank3 & data_.ChrMask];
        state_.PpuBanks[2] = &block[state_.ChrBank4 & data_.ChrMask];
        state_.PpuBanks[3] = &block[state_.ChrBank5 & data_.ChrMask];

        if ((state_.ChrMode & 2) != 0)
        {
            // RAMBO-1 full 1kb mode
            state_.PpuBanks[4] = &block[state_.ChrBank0 & data_.ChrMask];
            state_.PpuBanks[5] = &block[state_.ChrBank6 & data_.ChrMask];
            state_.PpuBanks[6] = &block[state_.ChrBank1 & data_.ChrMask];
            state_.PpuBanks[7] = &block[state_.ChrBank7 & data_.ChrMask];
        }
        else
        {
            auto base0 = &block[state_.ChrBank0 & data_.ChrMask & 0xfffff800];
            state_.PpuBanks[4] = base0;
            state_.PpuBanks[5] = base0 + 0x400;

            auto base1 = &block[state_.ChrBank1 & data_.ChrMask & 0xfffff800];
            state_.PpuBanks[6] = base1;
            state_.PpuBanks[7] = base1 + 0x400;
        }

        if (mapper_ == MapperType::TxSROM || mapper_ == MapperType::Tengen800037)
        {
            auto base = bus.GetPpuRamBase();
            state_.PpuBanks[8] = state_.PpuBanks[12] = &base[(state_.ChrBank2 >> 7) & 0x00400];
            state_.PpuBanks[9] = state_.PpuBanks[13] = &base[(state_.ChrBank3 >> 7) & 0x00400];
            state_.PpuBanks[10] = state_.PpuBanks[14] = &base[(state_.ChrBank4 >> 7) & 0x00400];
            state_.PpuBanks[11] = state_.PpuBanks[15] = &base[(state_.ChrBank5 >> 7) & 0x00400];
        }
    }
}

void MMC3::Write2(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
}

void MMC3::A12Rising(Bus& bus, CartCoreState& state, CartData& data)
{
    auto& mmc3State = state.MapperState.MMC3;

    if (state.ChrA12Sensitivity == ChrA12Sensitivity::RisingEdgeSmoothed)
    {
        // MMC3 takes a smoothed signal, and clocks on the rising edge of A12
        ClockIrqCounter(bus, state);
    }
}

void MMC3::UpdateChrA12Sensitivity(Bus& bus, CartCoreState& state, ChrA12Sensitivity sensitivity)
{
    auto mmc3State = state.MapperState.MMC3;

    auto sensitivityBefore = state.ChrA12Sensitivity;
    if (state.IrqCounter > 0 || (mmc3State.ReloadValue > 0) || mmc3State.IrqEnabled)
        state.ChrA12Sensitivity = sensitivity;
    else
        state.ChrA12Sensitivity = ChrA12Sensitivity::None;

    if (state.ChrA12Sensitivity != sensitivityBefore)
        bus.UpdateA12Sensitivity();
}
