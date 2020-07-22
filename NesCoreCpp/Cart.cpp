#include "Cart.h"

#include "Bus.h"

#include <assert.h>
#include <memory>

Cart::Cart() :
    cpuBanks_{},
    ppuBanks_{},
    chrWriteable_{ false },
    mapper_{ 0 },
    mapperShift_{ 0 },
    mapperShiftCount_{ 0 },
    prgMode_{ 3 },
    prgBank_{ 0 },
    chrMode_{ 0 },
    chrBank0_{ 0 },
    chrBank1_{ 0 },
    chrA12_{ false },
    prgPlane0_{ 0 },
    prgPlane1_{ 0 },
    prgRamBank0_{ 0 },
    prgRamBank1_{ 0 },
    chrA12Sensitive_{ false }
{
    ppuRamAddressMap_ = { 0, 0, 0x400, 0x400 };
}

void Cart::SetMapper(int mapper)
{
    mapper_ = mapper;
}

void Cart::SetPrgRom(std::vector<uint8_t> prgData)
{
    prgData_ = std::move(prgData);

    // first and last bank mapped by default.
    cpuBanks_[4] = &prgData_[0];
    cpuBanks_[5] = &prgData_[0x2000];
    cpuBanks_[6] = &prgData_[prgData_.size() - 0x4000];
    cpuBanks_[7] = &prgData_[prgData_.size() - 0x2000];

    prgMask16k_ = static_cast<uint32_t>(prgData_.size()) - 1;
    prgMask32k_ = prgMask16k_ & 0xffff8000;
}

void Cart::AddPrgRam()
{
    localPrgRam_.resize(0x2000);
    prgRamBanks_.push_back(&localPrgRam_[0]);
    cpuBanks_[3] = prgRamBanks_[0];
}

void Cart::AddPrgRam(uint8_t* data)
{
    prgRamBanks_.push_back(data);
    cpuBanks_[3] = prgRamBanks_[0];
}

void Cart::SetChrRom(std::vector<uint8_t> chrData)
{
    chrData_ = std::move(chrData);

    ppuBanks_[0] = &chrData_[0];
    ppuBanks_[1] = &chrData_[0x0400];
    ppuBanks_[2] = &chrData_[0x0800];
    ppuBanks_[3] = &chrData_[0x0c00];
    ppuBanks_[4] = &chrData_[0x1000];
    ppuBanks_[5] = &chrData_[0x1400];
    ppuBanks_[6] = &chrData_[0x1800];
    ppuBanks_[7] = &chrData_[0x1c00];

    assert((chrData_.size() & (chrData_.size() - 1)) == 0);
    chrMask_ = static_cast<uint32_t>(chrData_.size()) - 1;
}

void Cart::SetChrRam()
{
    chrData_.resize(0x2000);

    ppuBanks_[0] = &chrData_[0];
    ppuBanks_[1] = &chrData_[0x0400];
    ppuBanks_[2] = &chrData_[0x0800];
    ppuBanks_[3] = &chrData_[0x0c00];
    ppuBanks_[4] = &chrData_[0x1000];
    ppuBanks_[5] = &chrData_[0x1400];
    ppuBanks_[6] = &chrData_[0x1800];
    ppuBanks_[7] = &chrData_[0x1c00];

    chrWriteable_ = true;
}

void Cart::SetMirrorMode(bool verticalMirroring)
{
    if (verticalMirroring)
        ppuRamAddressMap_ = { 0, 0x400, 0, 0x400 };
    else
        ppuRamAddressMap_ = { 0, 0, 0x400, 0x400 };
}

void Cart::Attach(Bus* bus)
{
    bus_ = bus;
}

uint8_t Cart::CpuRead(uint16_t address) const
{
    auto bank = cpuBanks_[address >> 13];

    if (bank == nullptr)
        return 0;

    return bank[address & 0x1fff];
}

void Cart::CpuWrite(uint16_t address, uint8_t value)
{
    if (address < 0x8000)
    {
        auto bank = cpuBanks_[address >> 13];
        if (bank)
            bank[address & 0x1fff] = value;
        return;
    }

    switch (mapper_)
    {
    case 1:
        WriteMMC1(address, value);
        break;

    case 2:
        WriteUxROM(address, value);
        break;

    case 3:
        WriteCNROM(address, value);
        break;

    case 4:
        WriteMMC3(address, value);
        break;
    }
}

void Cart::CpuWrite2(uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
    if (address < 0x8000)
    {
        bus_->TickCpuWrite();

        auto bank = cpuBanks_[address >> 13];
        if (bank)
            bank[address & 0x1fff] = secondValue;
        return;
    }

    switch (mapper_)
    {
    case 1:
        // The MMC1 takes the first value and ignores the second.
        WriteMMC1(address, firstValue);
        bus_->TickCpuWrite();
        break;

    case 2:
        bus_->TickCpuWrite();
        WriteUxROM(address, secondValue);
        break;

    case 3:
        WriteCNROM(address, firstValue);
        bus_->TickCpuWrite();
        WriteCNROM(address, secondValue);
        break;

    case 4:
        WriteMMC3(address, firstValue);
        bus_->TickCpuWrite();
        WriteMMC3(address, secondValue);
        break;

    default:
        bus_->TickCpuWrite();
        break;
    }
}

uint8_t Cart::PpuRead(uint16_t address) const
{
    //assert(((address & 0x1000) != 0) == chrA12_);

    auto bank = ppuBanks_[address >> 10];
    return bank[address & 0x03ff];
}

uint16_t Cart::PpuReadChr16(uint16_t address) const
{
    //assert(((address & 0x1000) != 0) == chrA12_);

    auto bank = ppuBanks_[address >> 10];
    auto bankAddress = address & 0x03ff;
    return (bank[bankAddress] << 8) | bank[bankAddress | 8];
}

void Cart::PpuWrite(uint16_t address, uint8_t value)
{
    //assert(((address & 0x1000) != 0) == chrA12_);

    if (chrWriteable_)
    {
        auto bank = ppuBanks_[address >> 10];
        bank[address & 0x03ff] = value;
    }
}

uint16_t Cart::EffectivePpuRamAddress(uint16_t address) const
{
    auto page = (address >> 10) & 0x03;
    return ppuRamAddressMap_[page] | (address & 0x3ff);
}

bool Cart::SensitiveToChrA12()
{
    return chrA12Sensitive_;
}

void Cart::SetChrA12(bool set)
{
    if (chrA12Sensitive_)
    {
        if (set != chrA12_)
        {
            chrA12_ = set;
            UpdatePrgMapMMC1();

            if (cpuBanks_[3])
                cpuBanks_[3] = chrA12_ ? prgRamBanks_[prgRamBank1_] : prgRamBanks_[prgRamBank0_];
        }
        return;
    }

    chrA12_ = set;
}

void Cart::WriteMMC1(uint16_t address, uint8_t value)
{
    if ((value & 0x80) == 0x80)
    {
        mapperShiftCount_ = 0;
        mapperShift_ = 0;

        // when the shift register is reset, the control bit is or'd with 0x0C
        prgMode_ = 3;
        UpdatePrgMapMMC1();
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
        UpdateChrMapMMC1();

        // prg mode
        prgMode_ = (value >> 2) & 0x03;
        UpdatePrgMapMMC1();

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
        UpdateChrMapMMC1();

        chrA12Sensitive_ = false;

        // TODO: SNROM PRG RAM disable line
        if (prgData_.size() == 0x80000)
        {
            prgPlane0_ = ((chrBank0_ << 2) & 0x40000);
            chrA12Sensitive_ |= prgPlane0_ != prgPlane1_;

            if (!chrA12_)
                UpdatePrgMapMMC1();
        }

        if (prgRamBanks_.size() > 1)
        {
            if (prgRamBanks_.size() > 2)
                prgRamBank0_ = (value >> 2) & 0x03;
            else
                prgRamBank0_ = (value >> 1) & 0x01;

            chrA12Sensitive_ |= prgRamBank0_ != prgRamBank1_;
            if (!chrA12_ && cpuBanks_[3])
                cpuBanks_[3] = prgRamBanks_[prgRamBank0_];
        }

        break;

    case 6:
        // CHR bank 1
        chrBank1_ = value << 12;
        UpdateChrMapMMC1();

        chrA12Sensitive_ = false;

        // TODO: SNROM PRG RAM disable line
        if (prgData_.size() == 0x80000)
        {
            prgPlane1_ = ((chrBank1_ << 2) & 0x40000);
            chrA12Sensitive_ |= prgPlane0_ != prgPlane1_;

            if (chrA12_)
                UpdatePrgMapMMC1();
        }

        // TODO: SZROM maps this differently.
        if (prgRamBanks_.size() > 1)
        {
            if (prgRamBanks_.size() > 2)
                prgRamBank1_ = (value >> 2) & 0x03;
            else
                prgRamBank1_ = (value >> 1) & 0x01;

            chrA12Sensitive_ |= prgRamBank0_ != prgRamBank1_;
            if (chrA12_ && cpuBanks_[3])
                cpuBanks_[3] = prgRamBanks_[prgRamBank1_];
        }

        break;

    case 7:
        // PRG bank

        // enable/disable PRG RAM
        if (value & 0x10 || prgRamBanks_.size() == 0)
            cpuBanks_[3] = nullptr;
        else
            cpuBanks_[3] = chrA12_ ? prgRamBanks_[prgRamBank1_] : prgRamBanks_[prgRamBank0_];

        prgBank_ = (value & 0x0f) << 14;
        UpdatePrgMapMMC1();
        break;
    }
}

void Cart::UpdateChrMapMMC1()
{
    bus_->SyncPpu();

    switch (chrMode_)
    {
    case 0:
    {
        auto chrBank = static_cast<size_t>(chrBank0_) & 0x1e000;
        auto base = &chrData_[chrBank];
        ppuBanks_[0] = base;
        ppuBanks_[1] = base + 0x0400;
        ppuBanks_[2] = base + 0x0800;
        ppuBanks_[3] = base + 0x0c00;
        ppuBanks_[4] = base + 0x1000;
        ppuBanks_[5] = base + 0x1400;
        ppuBanks_[6] = base + 0x1800;
        ppuBanks_[7] = base + 0x1c00;
        break;
    }

    case 1:
    {
        auto base0 = &chrData_[chrBank0_ & (chrData_.size() - 1)];
        ppuBanks_[0] = base0;
        ppuBanks_[1] = base0 + 0x0400;
        ppuBanks_[2] = base0 + 0x0800;
        ppuBanks_[3] = base0 + 0x0c00;

        auto base1 = &chrData_[chrBank1_ & (chrData_.size() - 1)];
        ppuBanks_[4] = base1;
        ppuBanks_[5] = base1 + 0x0400;
        ppuBanks_[6] = base1 + 0x0800;
        ppuBanks_[7] = base1 + 0x0c00;
        break;
    }
    }
}

void Cart::UpdatePrgMapMMC1()
{
    auto prgPlane = chrA12_ ? prgPlane1_ : prgPlane0_;

    switch (prgMode_)
    {
    case 0:
    case 1:
    {
        auto base = &prgData_[prgPlane | (prgBank_ & prgMask32k_)];
        cpuBanks_[4] = base;
        cpuBanks_[5] = base + 0x2000;
        cpuBanks_[6] = base + 0x4000;
        cpuBanks_[7] = base + 0x6000;
        break;
    }

    case 2:
    {
        auto base = &prgData_[prgPlane | (prgBank_ & prgMask16k_)];
        cpuBanks_[4] = &prgData_[prgPlane];
        cpuBanks_[5] = &prgData_[prgPlane | 0x2000];
        cpuBanks_[6] = base;
        cpuBanks_[7] = base + 0x2000;
        break;
    }

    case 3:
    {
        auto base = &prgData_[prgPlane | (prgBank_ & prgMask16k_)];
        cpuBanks_[4] = base;
        cpuBanks_[5] = base + 0x2000;
        cpuBanks_[6] = &prgData_[prgPlane | ((prgData_.size() - 0x4000) & 0x3ffff)];
        cpuBanks_[7] = &prgData_[prgPlane | ((prgData_.size() - 0x2000) & 0x3ffff)];
    }
    }
}

void Cart::WriteUxROM(uint16_t address, uint8_t value)
{
    // TODO: bus conflicts for older carts
    auto bankAddress = (value << 14) & prgMask16k_;
    auto base = &prgData_[bankAddress];
    cpuBanks_[4] = base;
    cpuBanks_[5] = base + 0x2000;
}

void Cart::WriteCNROM(uint16_t address, uint8_t value)
{
    // TODO: bus conflicts
    // the actual board only takes 2 bits from the value for bank switching
    auto bankAddress = (value << 13) & chrMask_;

    auto base = &chrData_[bankAddress];

    if (base != ppuBanks_[0])
    {
        bus_->SyncPpu();
        ppuBanks_[0] = base;
        ppuBanks_[1] = base + 0x0400;
        ppuBanks_[2] = base + 0x0800;
        ppuBanks_[3] = base + 0x0c00;
        ppuBanks_[4] = base + 0x1000;
        ppuBanks_[5] = base + 0x1400;
        ppuBanks_[6] = base + 0x1800;
        ppuBanks_[7] = base + 0x1c00;
    }
}

void Cart::WriteMMC3(uint16_t address, uint8_t value)
{
    if (address < 0xa000)
    {
        if ((address & 1) == 0)
        {
            prgBank_ = value & 0x07;
            SetPrgModeMMC3((value >> 6) & 1);
            SetChrModeMMC3((value >> 7) & 1);
        }
        else
        {
            SetBankMMC3(value);
        }
    }
}

void Cart::SetPrgModeMMC3(uint8_t mode)
{
    if (mode != prgMode_)
    {
        std::swap(cpuBanks_[4], cpuBanks_[6]);

        prgMode_ = mode;
    }
}

void Cart::SetChrModeMMC3(uint8_t mode)
{
    if (mode != chrMode_)
    {
        std::swap(ppuBanks_[0], ppuBanks_[4]);
        std::swap(ppuBanks_[1], ppuBanks_[5]);
        std::swap(ppuBanks_[2], ppuBanks_[6]);
        std::swap(ppuBanks_[3], ppuBanks_[7]);

        chrMode_ = mode;
    }
}

void Cart::SetBankMMC3(uint32_t bank)
{
    switch (prgBank_)
    {
    case 0:
        bus_->SyncPpu();
        SetChrBank2k(chrMode_ ? 4 : 0, bank);
        break;

    case 1:
        bus_->SyncPpu();
        SetChrBank2k(chrMode_ ? 6 : 2, bank);
        break;

    case 2:
        bus_->SyncPpu();
        SetChrBank1k(chrMode_ ? 0 : 4, bank);
        break;

    case 3:
        bus_->SyncPpu();
        SetChrBank1k(chrMode_ ? 1 : 5, bank);
        break;

    case 4:
        bus_->SyncPpu();
        SetChrBank1k(chrMode_ ? 2 : 6, bank);
        break;

    case 5:
        bus_->SyncPpu();
        SetChrBank1k(chrMode_ ? 3 : 7, bank);
        break;

    case 6:
        cpuBanks_[prgMode_ ? 6 : 4] = &prgData_[(bank << 13) & prgMask16k_];
        break;

    case 7:
        cpuBanks_[5] = &prgData_[(bank << 13) & prgMask16k_];
        break;
    }
}

void Cart::SetChrBank1k(uint32_t bank, uint32_t value)
{
    auto bankAddress = (value << 10) & chrMask_;
    auto base = &chrData_[bankAddress];
    ppuBanks_[bank] = base;
}

void Cart::SetChrBank2k(uint32_t bank, uint32_t value)
{
    auto bankAddress = (value << 10) & chrMask_ & 0xfffff800;
    auto base = &chrData_[bankAddress];
    ppuBanks_[bank] = base;
    ppuBanks_[bank + 1] = base + 0x0400;
}

std::unique_ptr<Cart> TryCreateCart(
    const CartDescriptor& desc,
    std::vector<uint8_t> prgData,
    std::vector<uint8_t> chrData,
    uint8_t* batteryRam)
{
    auto cart = std::make_unique<Cart>();

    if (prgData.size() & (prgData.size() - 1))
        return nullptr; // non-zero power of 2 size expected

    cart->SetPrgRom(std::move(prgData));

    if (chrData.size() != 0)
    {
        cart->SetChrRom(std::move(chrData));
    }

    switch (desc.MirrorMode)
    {
    case MirrorMode::Horizontal:
        cart->SetMirrorMode(false);
        break;

    case MirrorMode::Vertical:
        cart->SetMirrorMode(true);
        break;

    case MirrorMode::FourScreen:
        // TODO:
        return nullptr;
    }

    if (desc.Mapper > 4)
        return nullptr;

    cart->SetMapper(desc.Mapper);

    if (desc.PrgRamSize != 0 && desc.PrgRamSize != 0x2000)
        return nullptr;

    if (desc.PrgBatteryRamSize != 0 && desc.PrgBatteryRamSize != 0x2000)
        return nullptr;

    if (desc.PrgRamSize != 0)
        cart->AddPrgRam();

    if (desc.PrgBatteryRamSize != 0)
    {
        if (batteryRam)
            cart->AddPrgRam(batteryRam);
        else
            cart->AddPrgRam();
    }

    if (desc.ChrRamSize != 0 && desc.ChrRamSize != 0x2000)
        return nullptr;

    if (desc.ChrRamSize != 0)
        cart->SetChrRam();

    return std::move(cart);
}