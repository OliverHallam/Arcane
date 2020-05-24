#pragma once

#include <cstdint>
#include <string>

struct CartRecord
{
    std::string Name;
    uint32_t PrgRom{};
    uint32_t ChrRom{};
    uint32_t MiscRom{};
    uint16_t Mapper{};
    uint8_t SubMapper{};
    std::string MirrorFlag;
    std::string Ppu;
    std::uint32_t PrgRam{};
    std::uint32_t PrgRamB{};
    std::uint32_t ChrRam{};
    std::uint32_t ChrRamB{};
    std::string Console;
    std::string Controller;
    uint32_t Crc32{};
};