#pragma once

#include "CartDescriptor.h"
#include "CartCoreState.h"
#include "CartState.h"

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

    void SetChrBank1k(uint32_t bank, uint32_t value);
    void SetChrBank2k(uint32_t bank, uint32_t value);

    void UpdatePpuRamMap();

    void SetChrA12Impl(bool set);

    Bus* bus_;

    uint32_t mapper_;

    std::vector<uint8_t> prgData_;
    std::vector<uint8_t> chrData_;

    uint32_t prgMask32k_;
    uint32_t prgMask16k_;

    uint32_t chrMask_;

    CartCoreState state_;

    std::vector<uint8_t> localPrgRam_;
    std::vector<uint8_t*> prgRamBanks_;

    bool chrWriteable_;
};

std::unique_ptr<Cart> TryCreateCart(
    const CartDescriptor& desc,
    std::vector<uint8_t> prgData,
    std::vector<uint8_t> chrData,
    uint8_t* batteryRam);
