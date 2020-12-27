#pragma once

#include "CartDescriptor.h"
#include "CartCoreState.h"
#include "CartState.h"
#include "ChrA12Sensitivity.h"
#include "MapperType.h"


#include <cstdint>
#include <memory>
#include <array>
#include <vector>

class Bus;

class Cart
{
public:
    Cart();

    void SetMapper(MapperType mapper);
    void SetPrgRom(std::vector<uint8_t> prgData);
    void AddPrgRam();
    void AddPrgRam(uint8_t* data);
    void SetChrRom(std::vector<uint8_t> chrData);
    void SetChrRam();
    void SetMirrorMode(MirrorMode mirrorMode);
    void EnableBusConflicts(bool conflicts);

    void Attach(Bus* bus);

    uint8_t CpuRead(uint16_t address);
    void CpuWrite(uint16_t address, uint8_t value);
    void CpuWrite2(uint16_t address, uint8_t firstValue, uint8_t secondValue);

    uint8_t PpuRead(uint16_t address);
    uint16_t PpuReadChr16(uint16_t address);
    void PpuDummyTileFetch();
    void PpuSpriteNametableFetch();
    void PpuWrite(uint16_t address, uint8_t value);

    ChrA12Sensitivity ChrA12Sensitivity() const;
    void ChrA12Rising();
    void ChrA12Falling();
    uint32_t A12PulsesUntilSync();

    bool HasScanlineCounter() const;
    void ScanlineCounterBeginScanline();
    void ScanlineCounterEndFrame();
    void TileSplitBeginScanline(bool firstTileIsAttribute);

    void InterceptWritePpuCtrl(bool largeSprites);
    void InterceptWritePpuMask(bool renderingEnabled);

    bool UsesMMC5Audio() const;

    void CaptureState(CartState* state) const;
    void RestoreState(const CartState& state);

private:
    void WriteMMC1(uint16_t address, uint8_t value);
    void WriteMMC1Register(uint16_t address, uint8_t value);
    void UpdateChrMapMMC1();
    void UpdatePrgMapMMC1();

    void WriteUxROM(uint16_t address, uint8_t value);

    void WriteCNROM(uint16_t address, uint8_t value);

    void WriteMMC3(uint16_t address, uint8_t value);
    void SetPrgModeMMC3(uint8_t mode);
    void SetChrModeMMC3(uint8_t mode);
    void SetBankMMC3(uint32_t bank);
    void ClockScanlineCounter();

    uint8_t ReadMMC5(uint16_t address);
    void WriteMMC5(uint16_t address, uint8_t value);
    void UpdatePrgMapMMC5();
    void UpdateChrMapMMC5();
    void UpdateNametableMapMMC5();
    void UpdateNametableMMC5(uint32_t index, uint8_t mode);

    void SetChrBank1k(uint32_t bank, uint32_t value);
    void SetChrBank2k(uint32_t bank, uint32_t value);

    void UpdatePpuRamMap();

    Bus* bus_;

    MapperType mapper_;

    std::vector<uint8_t> prgData_;
    std::vector<uint8_t> chrData_;

    uint32_t prgMask_;

    uint32_t chrMask_;

    CartCoreState state_;

    std::vector<uint8_t> localPrgRam_;
    std::vector<uint8_t*> prgRamBanks_;

    bool chrWriteable_;
    bool busConflicts_;
};

std::unique_ptr<Cart> TryCreateCart(
    const CartDescriptor& desc,
    std::vector<uint8_t> prgData,
    std::vector<uint8_t> chrData,
    uint8_t* batteryRam);
