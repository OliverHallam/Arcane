#pragma once

#include <cstdint>

enum class SyncEvent : uint32_t
{
    None,
    ApuFrameCounter,
    ApuSample,
    ApuSync,
    PpuScanline,
    PpuStateUpdate,
    PpuSyncA12,
    PpuSync,
    ScanlineCounterScanline,
    ScanlineCounterEndFrame,
    CpuNmi,
    CartCpuIrqCounter,
    CartSetIrq,
    CpuSetIrq,
    CpuClearIrq
};