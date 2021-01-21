#include "BusState.h"

BusState::BusState() :
    CycleCount{0},
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
