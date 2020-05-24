#pragma once

#include <cstdint>

class Crc32
{
public:
    Crc32();

    void AddData(const uint8_t* data, size_t length);

    uint32_t GetHash() const;

private:
    uint32_t lookupTable_[256];

    uint32_t hash_;
};