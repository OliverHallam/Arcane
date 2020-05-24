#pragma once

#include "MirrorMode.h"

#include <cstdint>

struct CartDescriptor
{
    uint16_t Mapper{};
    uint8_t SubMapper{};
    MirrorMode MirrorMode{};
    uint32_t PrgRamSize{};
    uint32_t PrgBatteryRamSize{};
    uint32_t ChrRamSize{};
};