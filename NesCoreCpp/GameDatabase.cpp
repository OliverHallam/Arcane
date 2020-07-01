#include "GameDatabase.h"

#include "CartData.h"
#include "Crc32.h"

#include <memory>

std::unique_ptr<CartDescriptor> GameDatabase::Lookup(const std::vector<std::uint8_t>& prgData, const std::vector<std::uint8_t>& chrData)
{
    auto crc = HashData(prgData, chrData);
    return Lookup(crc);
}

uint32_t GameDatabase::HashData(const std::vector<std::uint8_t>& prgData, const std::vector<std::uint8_t>& chrData)
{
    Crc32 crc;

    crc.AddData(&prgData[0], prgData.size());

    if (chrData.size())
        crc.AddData(&chrData[0], chrData.size());

    return crc.GetHash();
}

std::unique_ptr<CartDescriptor> GameDatabase::Lookup(uint32_t crc32)
{
    auto currentEntry = CartData;
    auto lastEntry = currentEntry + sizeof(CartData);

    // 6 bytes per game in the database
    for (; currentEntry < lastEntry; currentEntry += 6)
    {
        if (*reinterpret_cast<const uint32_t*>(currentEntry) == crc32)
            return DecodeDescriptor(currentEntry[4], currentEntry[5]);
    }

    return nullptr;
}

std::unique_ptr<CartDescriptor> GameDatabase::DecodeDescriptor(uint8_t mapperByte, uint8_t configByte)
{
    auto descriptor = std::make_unique<CartDescriptor>();

    descriptor->Mapper = mapperByte;

    // sssm mcbr
    // |||| |||+ 8K PRG-RAM
    // |||| ||+- 8K Battery backed PRG RAM
    // |||| |+-- 8K CHR RAM
    // |||+-+--- Mirror Mode
    // +++------ Sub mapper / prg-ram size override

    if (configByte & 0x01)
        descriptor->PrgRamSize = 8 * 1024;

    if (configByte & 0x02)
        descriptor->PrgBatteryRamSize = 8 * 1024;

    if (configByte & 0x04)
        descriptor->ChrRamSize = 8 * 1024;

    descriptor->MirrorMode = static_cast<MirrorMode>((configByte & 0x18) >> 3);

    // special cases
    auto subMapperBits = configByte >> 5;
    if (mapperByte == 5)
    {
        switch (subMapperBits)
        {
        case 1:
            descriptor->PrgBatteryRamSize = 1024;
            break;

        case 2:
            descriptor->PrgBatteryRamSize = 32 * 1024;
            break;
        }

        descriptor->SubMapper = 0;
    }
    else
    {
        descriptor->SubMapper = subMapperBits;
    }

    if (mapperByte == 13)
        descriptor->ChrRamSize = 16 * 1024;

    return std::move(descriptor);
}
