#pragma once

#include <cstdint>
#include <vector>

struct CartData
{
public:
    std::vector<uint8_t> PrgData;
    uint32_t PrgMask{};
    uint32_t PrgBlockSize{};

    std::vector<uint8_t> ChrData;
    uint32_t ChrMask{};
    uint32_t ChrBlockSize{};

    std::vector<uint8_t> LocalPrgRam;
    std::vector<uint8_t> LocalBatteryRam;
    std::vector<uint8_t*> PrgRamBanks;
};