#include "Rambo1.h"

#include "..\Bus.h"
#include "..\CartCoreState.h"

void Rambo1::Initialize(CartCoreState& state, CartData& data)
{
    MMC3::Initialize(state, data);

    state.MapperState.MMC3.PrgBank2 = 0;

    // we always care about A12, since we need to know the last time it dropped, in order to correctly set the reset flag when we switch on interrupts.
    // TODO: we could do this without scheduling all A12 events.
    state.ChrA12Sensitivity = ChrA12Sensitivity::RisingEdgeSmoothed;
}

void Rambo1::ClockIrqCounter(Bus& bus, CartCoreState& state)
{
    // TODO: we can absorb this into the scheduler.
    MMC3::ClockIrqCounter();

    auto rambo1State = state.MapperState.Rambo1;
    if (rambo1State.IrqMode == 1 && (rambo1State.IrqCounter > 0 || rambo1State.ReloadValue > 0 || rambo1State.IrqEnabled))
    {
        bus.Schedule(4, SyncEvent::CartCpuIrqCounter);
    }
}