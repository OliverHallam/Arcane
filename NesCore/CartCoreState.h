#pragma once

#include "ChrA12Sensitivity.h"
#include "MirrorMode.h"

#include "Mappers/MMC1.h"
#include "Mappers/UxROM.h"
#include "Mappers/CNROM.h"
#include "Mappers/MMC3.h"
#include "Mappers/MMC6.h"
#include "Mappers/MCACC.h"
#include "Mappers/Rambo1.h"

#include <array>
#include <cstdint>

class Bus;
struct CartCoreState;

typedef void(*CartWriteHandler)(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
typedef void(*CartWrite2Handler)(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue);

typedef uint8_t(*CartReadHandler)(CartCoreState& state, CartData& data, uint16_t address);

struct CartCoreState
{
    union {
        MMC1State MMC1;
        UxROMState UxROM;
        CNROMState CNROM;
        MMC3State MMC3;
        MMC6State MMC6;
        MCACCState MCACC;
        Rambo1State Rambo1;
    } MapperState {};

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

    bool IrqEnabled{};
    uint32_t IrqCounter{};

    bool CpuCounterEnabled{};
    uint32_t CpuCounterSyncCycle{};

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
    std::array<CartWriteHandler, 8> WriteMap{};
    std::array<CartWrite2Handler, 8> Write2Map{};

    std::array<CartReadHandler, 8> ReadMap{};

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
