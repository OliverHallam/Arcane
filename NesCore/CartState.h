#pragma once

#include "CartCoreState.h"

#include <array>
#include <cstdint>

struct CartState
{
    CartCoreState Core;

    // TODO: resize these dynamically
    std::array<uint8_t, 0x8000> PrgRamBank1;
    std::array<uint8_t, 0x8000> PrgRamBank2;
    std::array<uint8_t, 0x4000> ChrRam;
};
