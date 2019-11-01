#pragma once

#include <cstdint>
#include <memory>

class Cart
{
public:
    bool VerticalMirroring();

    uint8_t CpuRead(uint16_t address);

    uint8_t PpuRead(uint16_t address);
};

std::unique_ptr<Cart> TryLoadCart(uint8_t* data, size_t length);