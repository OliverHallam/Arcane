#include "Crc32.h"

#include <memory>

Crc32::Crc32()
    : lookupTable_{}
{
    const uint32_t polynomial = 0xEDB88320UL;

    for (uint32_t i = 0u; i < 256; i++)
    {
        auto lookupValue = i;

        for (auto j = 0; j < 8; j++)
        {
            auto carry = (lookupValue & 1) != 0;
            lookupValue >>= 1;

            if (carry)
                lookupValue ^= polynomial;
        }

        lookupTable_[i] = lookupValue;
    }

    hash_ = 0xffffffff;
}

void Crc32::AddData(const uint8_t* data, size_t length)
{
    auto end = data + length;
    while (data != end)
    {
        auto hashByte = hash_ & 0xff;
        hashByte ^= *data++;

        hash_ = (hash_ >> 8) ^ lookupTable_[hashByte];
    }
}

uint32_t Crc32::GetHash() const
{
    return hash_ ^ 0xffffffff;
}
