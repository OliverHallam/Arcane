#pragma once

#include <array>
#include <cstdint>

#include "EventQueue.h"

struct BusState
{
    BusState();

    std::array<uint8_t, 2048> CpuRam;
    std::array<uint8_t, 2048> PpuRam;

    uint32_t CycleCount;

    bool Dma;
    bool OamDma;
    bool DmcDma;

    uint16_t OamDmaAddress;
    uint16_t DmcDmaAddress;

    bool AudioIrq;
    bool CartIrq;

    EventQueue SyncQueue;
};