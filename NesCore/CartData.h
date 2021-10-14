#pragma once

#include <cstdint>
#include <vector>

struct CartData
{
public:
    std::vector<uint8_t> PrgData;
    std::vector<uint8_t> ChrData;

    std::vector<uint8_t> LocalPrgRam;
    std::vector<uint8_t> LocalBatteryRam;
    std::vector<uint8_t*> PrgRamBanks;
};