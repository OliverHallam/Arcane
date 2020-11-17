#pragma once

#include <array>
#include <cstdint>

struct CartState
{
    CartCoreState Core;

    std::array<uint8_t, 0x2000> PrgRamBank1;
    std::array<uint8_t, 0x2000> PrgRamBank2;
    std::array<uint8_t, 0x2000> ChrRam;
};
