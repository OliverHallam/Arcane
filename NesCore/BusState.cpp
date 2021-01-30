#include "BusState.h"

BusState::BusState() :
    CpuCycleCount{ 0 },
    PpuCycleCount{ 0 },
    Dma{},
    OamDma{},
    DmcDma{},
    DmcDmaAddress{},
    OamDmaAddress{},
    AudioIrq{ false },
    CartIrq{ false }
{
    CpuRam.fill(0xff);
    PpuRam.fill(0xff);
}
