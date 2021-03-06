#pragma once

#include "ChrA12Sensitivity.h"
#include "MirrorMode.h"

#include <array>
#include <cstdint>

struct CartCoreState
{
    MirrorMode MirrorMode{ MirrorMode::Horizontal };

    // MMC1 shift register
    uint32_t MapperShiftCount{};
    uint32_t MapperShift{};

    uint32_t BankSelect{};
    uint32_t CommandNumber{};

    uint32_t PrgMode{};
    uint32_t PrgMode2{};
    uint32_t PrgBank0{};
    uint32_t PrgBank1{};
    uint32_t PrgBank2{};
    uint32_t PrgBank3{};
    uint32_t PrgBankHighBits{};
    bool PrgBank0Ram{};
    bool PrgBank1Ram{};
    bool PrgBank2Ram{};

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
    uint32_t ChrBankHighBits{};
    uint8_t ChrFillValue{};
    uint8_t ChrFillAttributes{};
    uint8_t NametableMode0{};
    uint8_t NametableMode1{};
    uint8_t NametableMode2{};
    uint8_t NametableMode3{};
    bool UseSecondaryChr0{};
    bool UseSecondaryChr1{};
    bool UseSecondaryChrForData{};

    uint32_t PrgPlane0{};
    uint32_t PrgPlane1{};
    uint32_t PrgRamEnabled{};
    uint32_t PrgRamBank0{};
    uint32_t PrgRamBank1{};
    uint32_t PrgRamProtect0{};
    uint32_t PrgRamProtect1{};

    uint32_t ExtendedRamMode{};
    uint8_t ExtendedAttribute{};
    uint32_t ExtendedPatternAddress{};

    ChrA12Sensitivity ChrA12Sensitivity{};
    bool ChrA12{};

    bool IrqEnabled{};
    uint32_t IrqMode{};
    uint32_t IrqCounter{};
    uint32_t ChrA12PulseCounter{};
    uint32_t LastA12Cycle{};
    uint32_t PrescalerResetCycle{};
    bool BumpIrqCounter{};

    bool CpuCounterEnabled{};
    uint32_t CpuCounterSyncCycle{};

    bool ReloadCounter{};
    uint8_t ReloadValue{};

    bool InFrame{};
    bool PpuInFrame{}; // can lag behind the other InFrame
    uint32_t InterruptScanline{};
    bool IrqPending{};

    bool LargeSprites{};
    bool RenderingEnabled{};
    bool MMC5InSync{};

    bool SplitEnabled{};
    bool RightSplit{};
    bool InSprites{};
    uint32_t CurrentTile{};
    uint32_t SplitTile{};
    uint32_t SplitScroll{};
    uint32_t SplitY{};
    uint32_t SplitBank{};

    uint8_t MulitplierArg0{};
    uint8_t MulitplierArg1{};

    int InitializationState{};

    // TODO: we should reconstruct these on restore, so we can share/save the state
    // The CPU address space in 8k banks
    std::array<uint8_t*, 8> CpuBanks{};
    std::array<bool, 8> CpuBankWritable{};

    // The PPU address space in 1K banks
    std::array<uint8_t*, 16> PpuBanks{};
    std::array<bool, 16> PpuBankWritable{};

    std::array<uint8_t, 4> PpuBankFillBytes{};
    std::array<uint8_t, 4> PPuBankAttributeBytes{};

    std::array<uint8_t, 1024> ExtendedRam{};
};
