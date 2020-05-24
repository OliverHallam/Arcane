#pragma once

#include "CartDescriptor.h"

#include <cstdint>
#include <memory>
#include <vector>

class GameDatabase
{
public:
    static std::unique_ptr<CartDescriptor> Lookup(
        const std::vector<std::uint8_t>& prgData,
        const std::vector<std::uint8_t>& chrData);

private:
    static uint32_t HashData(
        const std::vector<std::uint8_t>& prgData,
        const std::vector<std::uint8_t>& chrData);

    static std::unique_ptr<CartDescriptor> Lookup(uint32_t crc32);

    static std::unique_ptr<CartDescriptor> DecodeDescriptor(uint8_t mapperByte, uint8_t configByte);
};