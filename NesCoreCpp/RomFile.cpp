#include "RomFile.h"

#include <vector>

RomFile::RomFile(CartDescriptor descriptor, std::vector<uint8_t> prgData, std::vector<uint8_t> chrData)
    : Descriptor(std::move(descriptor)),
    PrgData(std::move(prgData)),
    ChrData(std::move(chrData))
{
}

std::unique_ptr<RomFile> TryLoadINesFile(const uint8_t* data, size_t length)
{
    auto end = data + length;

    if (length < 16)
        return nullptr;

    if (data[0] != 'N' || data[1] != 'E' || data[2] != 'S' || data[3] != 0x1a)
        return nullptr;

    auto prgSize = data[4] * 0x4000;
    auto chrSize = data[5] * 0x2000;
    auto flags6 = data[6];


    auto mapper = flags6 >> 4;
    auto verticalMirroring = (flags6 & 0x01) != 0;
    auto batteryBacked = (flags6 & 0x02) != 0;
    auto hasTrainer = (flags6 & 0x04) != 0;
    auto fourScreen = (flags6 & 0x08) != 0;

    data += 16;

    if (hasTrainer)
        data += 512;

    auto prgEnd = data + prgSize;
    if (prgEnd > end)
        return nullptr;

    if (prgSize == 0)
    {
        return nullptr;
    }

    std::vector<uint8_t> prgData{ data, prgEnd };

    data = prgEnd;

    auto chrEnd = data + chrSize;
    if (chrEnd > end)
        return nullptr;

    std::vector<uint8_t> chrData;
    if (chrSize != 0)
    {
        chrData = std::vector<uint8_t>(data, chrEnd);
    }

    data = chrEnd;

    if (data != end)
        return nullptr;

    CartDescriptor descriptor;
    descriptor.Mapper = mapper;
    descriptor.SubMapper = 0;
    if (fourScreen)
        descriptor.MirrorMode = MirrorMode::FourScreen;
    else if (verticalMirroring)
        descriptor.MirrorMode = MirrorMode::Vertical;
    else
        descriptor.MirrorMode = MirrorMode::Horizontal;

    if (mapper == 1)
    {
        if (batteryBacked)
            descriptor.PrgBatteryRamSize = 0x2000;
        else
            descriptor.PrgRamSize = 0x2000;
    }

    if (!chrSize)
    {
        descriptor.ChrRamSize = 0x2000;
    }

    return std::make_unique<RomFile>(std::move(descriptor), std::move(prgData), std::move(chrData));
}
