#include "Cart.h"

#include <memory>

bool Cart::VerticalMirroring()
{
    return verticalMirroring_;
}

uint8_t Cart::CpuRead(uint16_t address)
{
    if (address >= 0xC000)
    {
        return prgBankB_[address & 0x3fff];
    }

    if (address >= 0x8000)
    {
        return prgBankA_[address & 0x3fff];
    }

    return 0;
}

uint8_t Cart::PpuRead(uint16_t address)
{
    return chrData_[address & 0x1fff];
}

Cart::Cart(
    std::vector<uint8_t> prgBankA,
    std::vector<uint8_t> prgBankB,
    std::vector<uint8_t> chrData,
    bool verticalMirroring) :
    verticalMirroring_{ verticalMirroring },
    prgBankA_{ std::move(prgBankA) },
    prgBankB_{ std::move(prgBankB) },
    chrData_{ std::move(chrData) }
{
}

std::unique_ptr<Cart> TryLoadCart(const uint8_t* data, size_t length)
{
    auto end = data + length;

    if (length < 16)
        return nullptr;

    if (data[0] != 'N' || data[1] != 'E' || data[2] != 'S' || data[3] != 0x1a)
        return nullptr;

    auto prgCount = data[4];
    auto chrSize = data[5] * 0x2000;

    auto flags6 = data[6];

    auto mapper = flags6 >> 4;
    auto verticalMirroring = (flags6 & 1) != 0;
    auto hasTrainer = (flags6 & 0x04) != 0;

    data += 16;

    if (hasTrainer)
        data += 512;

    std::vector<std::vector<uint8_t>> prgBanks(prgCount);
    for (auto i = 0; i < prgCount; i++)
    {
        auto bankEnd = data + 0x4000;

        if (bankEnd > end)
            return nullptr;

        std::vector<uint8_t> prgBank(data, bankEnd);
        prgBanks[i] = prgBank;
        data = bankEnd;
    }

    auto chrEnd = data + chrSize;
    if (chrEnd > end)
        return nullptr;

    std::vector<uint8_t> chrData(data, chrEnd);
    data = chrEnd;

    if (data != end)
    {
        return nullptr;
    }

    switch (mapper)
    {
    case 0:
        switch (prgBanks.size())
        {
        case 1:
            return std::make_unique<Cart>(prgBanks[0], prgBanks[0], chrData, verticalMirroring);

        case 2:
            return std::make_unique<Cart>(prgBanks[0], prgBanks[1], chrData, verticalMirroring);
        }
        break;
    }

    return nullptr;
}
