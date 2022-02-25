#include "MCACC.h"

#include "../Bus.h"
#include "../CartCoreState.h"

void MCACC::Initialize(CartCoreState& state, CartData& data)
{
    MMC3::Initialize(state, data);

    state.WriteMap[6] = Write0xcxxx;
    state.WriteMap[7] = Write0xexxx;

    auto& mcaccState = state.MapperState.MCACC;
    mcaccState.ChrA12PulseCounter = 1;
}

void MCACC::A12Falling(Bus& bus, CartCoreState& state)
{
    auto& mcaccState = state.MapperState.MCACC;

    if (state.ChrA12Sensitivity == ChrA12Sensitivity::FallingEdgeDivided)
    {
        // MC-ACC clocks on every falling edge of A12, and triggers the scanline counter every 8 times.
        mcaccState.ChrA12PulseCounter--;
        if (mcaccState.ChrA12PulseCounter == 0)
        {
#ifdef DIAGNOSTIC
            bus.MarkDiagnostic(0xff0000ff);
#endif 

            MMC3::ClockIrqCounter(bus, state);
            mcaccState.ChrA12PulseCounter = 8;
        }
    }
}

void MMC3::Write0xcxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto& mcaccState = state.MapperState.MCACC;

    if ((address & 1) == 0)
    {
        mcaccState.ReloadValue = value;
    }
    else
    {
        mcaccState.ReloadCounter = true;

        // MC-ACC:
        // "writing to 0xC001 resets the pulse counter"
        mcaccState.ChrA12PulseCounter = 1;

#ifdef DIAGNOSTIC
        bus_->MarkDiagnostic(0xff444444);
#endif
    }

    MMC3::UpdateChrA12Sensitivity(bus, state, ChrA12Sensitivity::FallingEdgeDivided);
}

void MCACC::Write0xexxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
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

    MMC3::UpdateChrA12Sensitivity(bus, state, ChrA12Sensitivity::FallingEdgeDivided);
}
