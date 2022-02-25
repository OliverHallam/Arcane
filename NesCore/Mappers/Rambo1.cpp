#include "Rambo1.h"

#include "MapperUtils.h"

#include "..\Bus.h"
#include "..\CartCoreState.h"

void Rambo1::Initialize(CartCoreState& state, CartData& data)
{
    MMC3::Initialize(state, data);

    state.WriteMap[4] = Write0x8xxx;
    state.WriteMap[5] = Write0xaxxx;
    state.WriteMap[6] = Write0xcxxx;
    state.WriteMap[7] = Write0xexxx;

    auto& rambo1State = state.MapperState.Rambo1;
    rambo1State.IrqMode = 0;
    rambo1State.LastA12Cycle = 0;
    rambo1State.PrescalerResetCycle = 0;
    rambo1State.BumpIrqCounter = false;

    rambo1State.PrgBank2 = 0;

    // we always care about A12, since we need to know the last time it dropped, in order to correctly set the reset flag when we switch on interrupts.
    // TODO: we could do this without scheduling all A12 events.
    state.ChrA12Sensitivity = ChrA12Sensitivity::RisingEdgeSmoothed;
}

void Rambo1::A12Rising(Bus& bus, CartCoreState& state, CartData& data)
{
    if (state.ChrA12Sensitivity == ChrA12Sensitivity::RisingEdgeSmoothed)
    {
        auto& rambo1State = state.MapperState.Rambo1;
        rambo1State.LastA12Cycle = bus.GetA12FallingEdgeCycleSmoothed();
        if (rambo1State.IrqMode != 0)
            return;

        ClockIrqCounter(bus, state);
    }
}

void Rambo1::Write0x8xxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto& rambo1State = state.MapperState.Rambo1;

    if ((address & 1) == 0)
    {
        bus.SyncPpu();
        rambo1State.BankSelect = value & 0x07;
        rambo1State.PrgMode = (value >> 6) & 1;
        rambo1State.ChrMode = (value >> 7) & 1;
        rambo1State.ChrMode |= (value & 0x20) >> 4;
        MMC3::UpdatePrgMap(state, data);
        MMC3::UpdateChrMap(state, data);
    }
    else
    {
        MMC3::SetBank(bus, state, data, value);
    }
}

void Rambo1::Write0xaxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
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
}

void Rambo1::Write0xcxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto& rambo1State = state.MapperState.Rambo1;

    if ((address & 1) == 0)
    {
        rambo1State.ReloadValue = value;

        if (rambo1State.IrqMode == 1)
        {
            auto cyclesSinceReset = bus.PpuCycleCount() - rambo1State.PrescalerResetCycle;
            cyclesSinceReset %= 12;

            bus.Deschedule(SyncEvent::CartCpuIrqCounter);
            if (rambo1State.IrqMode == 1 && 
                (rambo1State.IrqCounter > 0 || rambo1State.ReloadValue > 0 || rambo1State.IrqEnabled))
            {
                bus.SchedulePpu(12 - cyclesSinceReset, SyncEvent::CartCpuIrqCounter);
            }
        }
    }
    else
    {
        rambo1State.IrqMode = value & 1;
        rambo1State.ReloadCounter = true;

        bus.Deschedule(SyncEvent::CartCpuIrqCounter);

        if (rambo1State.IrqMode == 0)
        {
            // this won't trip in the current scanline, if it was not within 16 CPU cycles of the clock going low.
            // We'll emulate this by bumping the count by one when we reset it.
            // an exception to this rule is if the count is zero, and we are resetting it to zero, then this happening a scanline late isn't observable
            if (rambo1State.ReloadValue == 0)
            {
                rambo1State.BumpIrqCounter = false;
            }
            else
            {
                rambo1State.BumpIrqCounter = bus.PpuCycleCount() >= rambo1State.LastA12Cycle + 48;
            }
        }
        else
        {
            rambo1State.PrescalerResetCycle = bus.PpuCycleCount();

            if (rambo1State.IrqMode == 1 && 
                (rambo1State.IrqCounter > 0 || rambo1State.ReloadValue > 0 || rambo1State.IrqEnabled))
            {
                bus.Schedule(4, SyncEvent::CartCpuIrqCounter);
            }
        }

#ifdef DIAGNOSTIC
        bus_->MarkDiagnostic(0xff444444);
#endif
    }

    MMC3::UpdateChrA12Sensitivity(bus, state, ChrA12Sensitivity::RisingEdgeSmoothed);
}

void Rambo1::Write0xexxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
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

    MMC3::UpdateChrA12Sensitivity(bus, state, ChrA12Sensitivity::RisingEdgeSmoothed);
}

void MMC3::A12Rising(Bus& bus, CartCoreState& state, CartData& data)
{
    auto& rambo1State = state.MapperState.Rambo1;

    if (state.ChrA12Sensitivity == ChrA12Sensitivity::RisingEdgeSmoothed)
    {
        rambo1State.LastA12Cycle = bus.GetA12FallingEdgeCycleSmoothed();
        if (rambo1State.IrqMode != 0)
            return;

        ClockIrqCounter(bus, state);
    }
}

void Rambo1::ClockCpuIrqCounter(Bus& bus, CartCoreState& state)
{
    ClockIrqCounter(bus, state);

    auto& rambo1State = state.MapperState.Rambo1;

    if (rambo1State.IrqMode == 1 && (rambo1State.IrqCounter > 0 || rambo1State.ReloadValue > 0 || rambo1State.IrqEnabled))
    {
        bus.Schedule(4, SyncEvent::CartCpuIrqCounter);
    }
}

void Rambo1::ClockIrqCounter(Bus& bus, CartCoreState& state)
{
    // TODO: we can absorb this into the scheduler.
    auto& rambo1State = state.MapperState.Rambo1;

    if (rambo1State.IrqCounter == 0 || rambo1State.ReloadCounter)
    {
        rambo1State.IrqCounter = rambo1State.ReloadValue;

        // RAMBO-1 often skips a scanline before resetting - we'll emulate this by resetting earlier, but bumping the count.
        if (rambo1State.BumpIrqCounter)
            rambo1State.IrqCounter += 1;

        rambo1State.BumpIrqCounter = false;
        rambo1State.ReloadCounter = false;
}
    else
    {
        rambo1State.IrqCounter--;
    }

    if (rambo1State.IrqCounter == 0 && rambo1State.IrqEnabled)
    {
        // there is a 3 cycle delay for RAMBO-1 IRQs.
        bus.Schedule(3, SyncEvent::CartSetIrq);
    }
}