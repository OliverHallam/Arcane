#pragma once

#include "CartDescriptor.h"

#include <cstdint>
#include <memory>
#include <vector>

struct RomFile
{
    RomFile(CartDescriptor descriptor, std::vector<uint8_t> prgData, std::vector<uint8_t> chrData);

    CartDescriptor Descriptor;
    std::vector<uint8_t> PrgData;
    std::vector<uint8_t> ChrData;
};

std::unique_ptr<RomFile> TryLoadINesFile(const uint8_t* data, size_t length);