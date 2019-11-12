#pragma once

#include <cstdint>
#include <memory>
#include <vector>

class Cart
{
public:
    bool VerticalMirroring();

    uint8_t CpuRead(uint16_t address);

    uint8_t PpuRead(uint16_t address);

    Cart(
        std::vector<uint8_t> prgBankA,
        std::vector<uint8_t> prgBankB,
        std::vector<uint8_t> chrData,
        bool verticalMirroring);

private:
    std::vector<uint8_t> prgBankA_;
    std::vector<uint8_t> prgBankB_;
    std::vector<uint8_t> chrData_;

    bool verticalMirroring_;
};

std::unique_ptr<Cart> TryLoadCart(const uint8_t* data, size_t length);