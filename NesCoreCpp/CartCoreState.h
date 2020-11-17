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
    uint32_t PrgBank{};

    uint32_t ChrMode{};
    uint32_t ChrBank0{};
    uint32_t ChrBank1{};

    bool chrA12_{};
    uint32_t PrgPlane0{};
    uint32_t PrgPlane1{};
    uint32_t PrgRamBank0{};
    uint32_t PrgRamBank1{};

    bool ChrA12Sensitive{};

    bool IrqEnabled{};
    bool ReloadCounter{};
    uint32_t ScanlineCounter{};
    uint8_t ReloadValue{};

    // TODO: we should reconstruct these on restore, so we can share/save the state

    // The CPU address space in 8k banks
    std::array<uint8_t*, 8> CpuBanks{};

    // The PPU address space in 1K banks
    std::array<uint8_t*, 16> PpuBanks{};
};
