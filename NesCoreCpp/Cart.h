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
    void AddPrgRam(uint32_t size);
    void AddPrgRam(uint8_t* data, uint32_t size);
    void SetChrRom(std::vector<uint8_t> chrData);
    void AddChrRam(uint32_t size);
    void SetMirrorMode(MirrorMode mirrorMode);
    void EnableBusConflicts(bool conflicts);

    void Initialize();

    void Attach(Bus* bus);

    uint8_t CpuRead(uint16_t address);
    void CpuWrite(uint16_t address, uint8_t value);
    void CpuWrite2(uint16_t address, uint8_t firstValue, uint8_t secondValue);

    uint8_t PpuReadData(uint16_t address);
    uint8_t PpuReadNametable(uint16_t address);
    uint8_t PpuReadAttributes(uint16_t address);
    uint8_t PpuReadPatternLow(uint16_t address);
    uint8_t PpuReadPatternHigh(uint16_t address);
    uint8_t PpuReadSpritePatternLow(uint16_t address);
    uint8_t PpuReadSpritePatternHigh(uint16_t address);
    uint16_t PpuReadPattern16(uint16_t address);
    void PpuWrite(uint16_t address, uint8_t value);

    ChrA12Sensitivity ChrA12Sensitivity() const;
    void ChrA12Rising();
    void ChrA12Falling();
    uint32_t A12PulsesUntilSync();

    bool HasScanlineCounter() const;
    void ScanlineCounterBeginScanline();
    void ScanlineCounterEndFrame();
    void TileSplitBeginScanline();
    void TileSplitBeginTile(uint32_t tile);
    void TileSplitBeginSprites();
    void TileSplitEndSprites();

    void InterceptWritePpuCtrl(bool largeSprites);
    void InterceptWritePpuMask(bool renderingEnabled);

    bool UsesMMC5Audio() const;

    void ClockCpuIrqCounter();

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
    void SetBankMMC3(uint32_t bank);
    void UpdatePrgMapMMC3();
    void UpdateChrMapMMC3();
    void ClockMMC3IrqCounter();

    uint8_t ReadMMC5(uint16_t address);
    void WriteMMC5(uint16_t address, uint8_t value);
    void UpdatePrgMapMMC5();
    void MapPrgBankMMC5(bool isRam, int32_t index, uint8_t** bank, bool* writable);
    void UpdateChrMapMMC5();
    void UpdateNametableMapMMC5();
    void UpdateNametableMMC5(uint32_t index, uint8_t mode);

    void WriteAxROM(uint16_t address, uint8_t value);

    void WriteMMC2(uint16_t address, uint8_t value);
    void PpuReadMMC2(uint16_t address);
    void UpdateChrMapMMC2();

    void WriteColorDreams(uint16_t address, uint8_t value);

    void WriteCPROM(uint16_t address, uint8_t value);

    void WriteNINA001(uint16_t address, uint8_t value);

    void WriteBNROM(uint16_t address, uint8_t value);

    void WriteCaltron6in1Low(uint16_t address);
    void WriteCaltron6in1High(uint16_t address, uint8_t value);

    void WriteRumbleStationLow(uint16_t address, uint8_t value);
    void WriteRumbleStationHigh(uint16_t address, uint8_t value);

    void WriteQJLow(uint16_t address, uint8_t value);

    void WriteRambo1(uint16_t address, uint8_t value);

    void WriteGxROM(uint16_t address, uint8_t value);

    void WriteSunsoft4(uint16_t address, uint8_t value);
    void UpdatePrgMapSunsoft4();
    void UpdateChrMapSunsoft4();
    void UpdateNametableMapSunsoft4();

    void WriteSunsoftFME7(uint16_t address, uint8_t value);
    void UpdatePrgMapSunsoftFME7();
    void UpdateChrMapSunsoftFME7();

    void WriteNINA03(uint16_t address, uint8_t value);

    void WriteNesEvent(uint16_t address, uint8_t value);
    void WriteNesEventRegister(uint16_t address, uint8_t value);
    void UpdatePrgMapNesEvent();

    void UpdateChrMapTQROM();
    void Set2kBankTQROM(int index, uint32_t bank);
    void Set1kBankTQROM(int index, uint32_t bank);

    void WriteSachenSA008A(uint16_t address, uint8_t value);

    void WriteActiveEnterprises(uint16_t address, uint8_t value);
    void UpdatePrgMapActiveEnterprises();

    void UpdatePrgMap32k();
    void UpdateChrMap8k();

    void UpdatePpuRamMap();

    Bus* bus_;

    MapperType mapper_;

    std::vector<uint8_t> prgData_;
    std::vector<uint8_t> chrData_;

    uint32_t prgMask_;
    uint32_t prgBlockSize_;

    uint32_t chrMask_;
    uint32_t chrBlockSize_;

    CartCoreState state_;

    std::vector<uint8_t> localPrgRam_;
    std::vector<uint8_t*> prgRamBanks_;

    uint32_t prgRamMask_;

    int32_t chrRamStart_;
    uint32_t chrRamMask_;
    bool busConflicts_;
};

std::unique_ptr<Cart> TryCreateCart(
    const CartDescriptor& desc,
    std::vector<uint8_t> prgData,
    std::vector<uint8_t> chrData,
    uint8_t* batteryRam);
