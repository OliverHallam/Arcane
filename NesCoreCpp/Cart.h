#pragma once

#include "CartDescriptor.h"

#include <cstdint>
#include <memory>
#include <array>
#include <vector>

class Bus;

class Cart
{
public:
    Cart();

    void SetMapper(int mapper);
    void SetPrgRom(std::vector<uint8_t> prgData);
    void AddPrgRam();
    void AddPrgRam(uint8_t* data);
    void SetChrRom(std::vector<uint8_t> chrData);
    void SetChrRam();
    void SetMirrorMode(MirrorMode mirrorMode);

    void Attach(Bus* bus);

    uint8_t CpuRead(uint16_t address) const;
    void CpuWrite(uint16_t address, uint8_t value);
    void CpuWrite2(uint16_t address, uint8_t firstValue, uint8_t secondValue);

    uint8_t PpuRead(uint16_t address) const;
    uint16_t PpuReadChr16(uint16_t address) const;
    void PpuWrite(uint16_t address, uint8_t value);

    bool SensitiveToChrA12();
    void SetChrA12(bool set);

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

    void SetChrBank1k(uint32_t bank, uint32_t value);
    void SetChrBank2k(uint32_t bank, uint32_t value);

    void UpdatePpuMap();

    // The CPU address space in 8k banks
    std::array<uint8_t*, 8> cpuBanks_;

    // The PPU address space in 1K banks
    std::array<uint8_t*, 16> ppuBanks_;
    bool chrWriteable_;

    std::vector<uint8_t> localPrgRam_;
    std::vector<uint8_t*> prgRamBanks_;

    std::vector<uint8_t> prgData_;
    std::vector<uint8_t> chrData_;

    MirrorMode mirrorMode_;

    uint32_t mapper_;

    uint32_t mapperShiftCount_;
    uint32_t mapperShift_;

    uint32_t prgMode_;
    uint32_t prgBank_;
    uint32_t prgMask32k_;
    uint32_t prgMask16k_;

    uint32_t chrMode_;
    uint32_t chrBank0_;
    uint32_t chrBank1_;
    uint32_t chrMask_;

    bool chrA12_;
    uint32_t prgPlane0_;
    uint32_t prgPlane1_;
    uint32_t prgRamBank0_;
    uint32_t prgRamBank1_;

    bool chrA12Sensitive_;

    Bus* bus_;
};

std::unique_ptr<Cart> TryCreateCart(
    const CartDescriptor& desc,
    std::vector<uint8_t> prgData,
    std::vector<uint8_t> chrData,
    uint8_t* batteryRam);
