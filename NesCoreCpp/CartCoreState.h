#pragma once

#include "MirrorMode.h"

#include <array>
#include <cstdint>

struct CartCoreState
{
    MirrorMode MirrorMode{ MirrorMode::Horizontal };

    // MMC1 shift register
    uint32_t MapperShiftCount{};
    uint32_t MapperShift{};

    uint32_t PrgMode{};
    uint32_t PrgBank0{};
    uint32_t PrgBank1{};
    uint32_t PrgBank2{};
    uint32_t PrgBank3{};

    uint32_t ChrMode{};
    uint32_t ChrBank0{};
    uint32_t ChrBank1{};
    uint32_t ChrBank2{};
    uint32_t ChrBank3{};
    uint32_t ChrBank4{};
    uint32_t ChrBank5{};
    uint32_t ChrBank6{};
    uint32_t ChrBank7{};
    uint32_t SecondaryChrBank0{};
    uint32_t SecondaryChrBank1{};
    uint32_t SecondaryChrBank2{};
    uint32_t SecondaryChrBank3{};
    uint8_t ChrFillValue{};
    uint8_t ChrFillAttributes{};
    uint8_t NametableMode0{};
    uint8_t NametableMode1{};
    uint8_t NametableMode2{};
    uint8_t NametableMode3{};
    bool UseSecondaryChrForData{};

    bool ChrA12{};
    uint32_t PrgPlane0{};
    uint32_t PrgPlane1{};
    uint32_t PrgRamBank0{};
    uint32_t PrgRamBank1{};

    uint8_t PrgRamProtect{};

    uint32_t ExtendedRamMode{};

    bool ChrA12Sensitive{};

    bool IrqEnabled{};
    uint32_t ScanlineCounter{};

    bool ReloadCounter{};
    uint8_t ReloadValue{};

    bool InFrame{};
    bool PpuInFrame{}; // can lag behind the other InFrame
    uint32_t InterruptScanline{};
    bool IrqPending{};

    bool LargeSprites{};
    bool RenderingEnabled{};
    bool ChrMapNeedsUpdate{};
    int32_t ScanlinePpuReadCount{};

    // TODO: we should reconstruct these on restore, so we can share/save the state
    // The CPU address space in 8k banks
    std::array<uint8_t*, 8> CpuBanks{};
    // The PPU address space in 1K banks
    std::array<uint8_t*, 16> PpuBanks{};

    std::array<bool, 4> PpuBankFillBytes{};
    std::array<uint8_t, 4> PPuBankAttributeBytes{};

    std::array<uint8_t, 1024> ExtendedRam{};
};
