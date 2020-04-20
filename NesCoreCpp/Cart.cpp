#include "Cart.h"

#include <assert.h>
#include <memory>

uint8_t Cart::CpuRead(uint16_t address) const
{
    auto bank = cpuBanks_[address >> 13];

    if (bank == nullptr)
        return 0;

    return bank[address & 0x1fff];
}

void Cart::CpuWrite(uint16_t address, uint8_t value)
{
    switch (mapper_)
    {
    case 1:
        WriteMMC1(address, value);
        break;
    }
}

uint8_t Cart::PpuRead(uint16_t address) const
{
    auto bank = ppuBanks_[address >> 12];
    return bank[address & 0x0fff];
}

uint16_t Cart::PpuReadChr16(uint16_t address) const
{
    auto bank = ppuBanks_[address >> 12];
    auto bankAddress = address & 0x0fff;
    return (bank[bankAddress] << 8) | bank[bankAddress | 8];
}

uint16_t Cart::EffectivePpuRamAddress(uint16_t address) const
{
    auto page = (address >> 10) & 0x03;
    return ppuRamAddressMap_[page] | (address & 0x3ff);
}

Cart::Cart(
    uint32_t mapper,
    std::vector<uint8_t> prgData,
    std::vector<uint8_t> chrData,
    bool verticalMirroring) :
    prgData_{ std::move(prgData) },
    chrData_{ std::move(chrData) },
    mapper_{ mapper },
    mapperShift_{ mapper },
    mapperShiftCount_{ 0 },
    prgMode_{ 3 },
    prgBank_{ 0 },
    chrMode_{ 0 },
    chrBank0_{ 0 },
    chrBank1_{ 0 }
{
    //cpuBanks_[3] = RAM
    cpuBanks_[4] = &prgData_[0];
    cpuBanks_[5] = &prgData_[0x2000];

    cpuBanks_[6] = &prgData_[prgData_.size() - 0x4000];
    cpuBanks_[7] = &prgData_[prgData_.size() - 0x2000];

    ppuBanks_[0] = &chrData_[0];
    ppuBanks_[1] = &chrData_[0x1000];

    if (verticalMirroring)
        ppuRamAddressMap_ = { 0, 0x400, 0, 0x400 };
    else
        ppuRamAddressMap_ = { 0, 0, 0x400, 0x400 };
}

void Cart::WriteMMC1(uint16_t address, uint8_t value)
{
    if ((value & 0x80) == 0x80)
    {
        mapperShiftCount_ = 0;
        mapperShift_ = 0;
        return;
    }

    // TODO: ignore if two writes two cycles in a row (e.g. RMW instructions)
    mapperShift_ |= (value & 1) << mapperShiftCount_;
    mapperShiftCount_++;

    if (mapperShiftCount_ == 5)
    {
        WriteMMC1Register(address, mapperShift_);

        mapperShiftCount_ = 0;
        mapperShift_ = 0;
    }
}

void Cart::WriteMMC1Register(uint16_t address, uint8_t value)
{
    switch (address >> 13)
    {
    case 4:
        // control

        // chr mode
        chrMode_ = (value >> 4) & 0x01;
        UpdateChrMap();

        // prg mode
        prgMode_ = (value >> 2) & 0x03;
        UpdatePrgMap();

        // mirroring mode
        switch (value & 0x03)
        {
        case 0:
            ppuRamAddressMap_ = { 0, 0, 0, 0 };
            break;

        case 1:
            ppuRamAddressMap_ = { 0x400, 0x400, 0x400, 0x400 };
            break;

        case 2:
            ppuRamAddressMap_ = { 0, 0x400, 0, 0x400 };
            break;

        case 3:
            ppuRamAddressMap_ = { 0, 0, 0x400, 0x400 };
            break;
        }

        break;

    case 5:
        // CHR bank 0
        chrBank0_ = value << 12;
        chrBank0_ &= chrData_.size() - 1;
        UpdateChrMap();
        break;

    case 6:
        // CHR bank 1
        chrBank1_ = value << 12;
        chrBank1_ &= chrData_.size() - 1;
        UpdateChrMap();
        break;

    case 7:
        // PRG bank
        assert((value & 0x10) == 0); // RAM not yet supported
        prgBank_ = (value & 0x0f) << 14;
        UpdatePrgMap();
        break;
    }
}

void Cart::UpdateChrMap()
{
    switch (chrMode_)
    {
    case 0:
    {
        auto chrBank = static_cast<size_t>(chrBank0_) & 0x1e000;
        ppuBanks_[0] = &chrData_[chrBank];
        ppuBanks_[1] = &chrData_[chrBank + 0x1000];
        break;
    }

    case 1:
        ppuBanks_[0] = &chrData_[chrBank0_];
        ppuBanks_[1] = &chrData_[chrBank1_];
        break;
    }
}

void Cart::UpdatePrgMap()
{
    switch (prgMode_)
    {
    case 0:
    case 1:
    {
        auto base = &prgData_[prgBank_ & 0x8000];
        cpuBanks_[4] = base;
        cpuBanks_[5] = base + 0x2000;
        cpuBanks_[6] = base + 0x4000;
        cpuBanks_[7] = base + 0x6000;
        break;
    }

    case 2:
        cpuBanks_[4] = &prgData_[0];
        cpuBanks_[5] = &prgData_[0x2000];
        cpuBanks_[6] = &prgData_[prgBank_];
        cpuBanks_[7] = &prgData_[prgBank_ + 0x2000];

    case 3:
        cpuBanks_[4] = &prgData_[prgBank_];
        cpuBanks_[5] = &prgData_[prgBank_ + 0x2000];
        cpuBanks_[6] = &prgData_[prgData_.size() - 0x4000];
        cpuBanks_[7] = &prgData_[prgData_.size() - 0x2000];
    }
}

std::unique_ptr<Cart> TryLoadCart(const uint8_t* data, size_t length)
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
    auto verticalMirroring = (flags6 & 1) != 0;
    auto hasTrainer = (flags6 & 0x04) != 0;

    data += 16;

    if (hasTrainer)
        data += 512;

    auto prgEnd = data + prgSize;
    if (prgEnd > end)
        return nullptr;

    std::vector<uint8_t> prgData;
    if (prgSize == 0)
    {
        prgData.resize(0x4000);
    }
    else
    {
        prgData = std::vector<uint8_t>(data, prgEnd);
    }

    data = prgEnd;

    auto chrEnd = data + chrSize;
    if (chrEnd > end)
        return nullptr;

    std::vector<uint8_t> chrData;
    if (chrSize == 0)
    {
        chrData.resize(0x2000);
    }
    else
    {
        chrData = std::vector<uint8_t>(data, chrEnd);
    }

    data = chrEnd;

    if (data != end)
    {
        return nullptr;
    }

    switch (mapper)
    {
    case 0:
    case 1:
        return std::make_unique<Cart>(mapper, prgData, chrData, verticalMirroring);
        break;
    }

    return nullptr;
}
