#pragma once

#include <cstdint>
#include <memory>
#include <array>
#include <vector>

class Cart
{
public:
    uint8_t CpuRead(uint16_t address) const;
    void CpuWrite(uint16_t address, uint8_t value);

    uint8_t PpuRead(uint16_t address) const;
    uint16_t PpuReadChr16(uint16_t address) const;

    uint16_t EffectivePpuRamAddress(uint16_t address) const;

    Cart(
        uint32_t mapper,
        std::vector<uint8_t> prgData,
        std::vector<uint8_t> chrData,
        bool verticalMirroring);

private:
    void WriteMMC1(uint16_t address, uint8_t value);
    void WriteMMC1Register(uint16_t address, uint8_t value);

    void UpdateChrMap();
    void UpdatePrgMap();

    // The CPU address space in 8k banks
    std::array<uint8_t*, 8> cpuBanks_;

    // The PPU address space in 4K banks
    std::array<uint8_t*, 2> ppuBanks_;

    std::vector<uint8_t> prgData_;
    std::vector<uint8_t> chrData_;

    std::array<uint16_t, 4> ppuRamAddressMap_;

    uint32_t mapper_;

    uint32_t mapperShiftCount_;
    uint32_t mapperShift_;

    uint32_t prgMode_;
    uint32_t prgBank_;

    uint32_t chrMode_;
    uint16_t chrBank0_;
    uint16_t chrBank1_;
};

std::unique_ptr<Cart> TryLoadCart(const uint8_t* data, size_t length);