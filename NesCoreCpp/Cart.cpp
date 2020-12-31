#include "Cart.h"

#include "Bus.h"

#include <assert.h>
#include <memory>

Cart::Cart() :
    chrWriteable_{ false },
    mapper_{ 0 },
    bus_{},
    chrMask_{},
    prgMask_{},
    busConflicts_{},
    prgRamMask_{}
{
}

void Cart::SetMapper(MapperType mapper)
{
    mapper_ = mapper;

    if (mapper_ == MapperType::MMC1)
        state_.PrgMode = 3;
    else if (mapper_ == MapperType::MMC5)
    {
        state_.PrgMode = 3;
        state_.PrgRamProtect0 = 3; // two protect flags
    }
    else if (mapper_ == MapperType::MCACC)
    {
        state_.ChrA12PulseCounter = 1;
    }
    else if (mapper_ == MapperType::AxROM || mapper_ == MapperType::ColorDreams || mapper_ == MapperType::Caltron6in1)
    {
        state_.CpuBanks[4] = &prgData_[0];
        state_.CpuBanks[5] = &prgData_[0x2000];
        state_.CpuBanks[6] = &prgData_[0x4000];
        state_.CpuBanks[7] = &prgData_[0x6000];
    }
    else if (mapper_ == MapperType::MMC2)
    {
        state_.CpuBanks[4] = &prgData_[0];
        state_.CpuBanks[5] = &prgData_[prgData_.size() - 0x6000];
        state_.CpuBanks[6] = &prgData_[prgData_.size() - 0x4000];
        state_.CpuBanks[7] = &prgData_[prgData_.size() - 0x2000];
    }
}

void Cart::SetPrgRom(std::vector<uint8_t> prgData)
{
    prgData_ = std::move(prgData);

    // first and last bank mapped by default.
    state_.CpuBanks[4] = &prgData_[0];
    state_.CpuBanks[5] = &prgData_[0x2000];
    state_.CpuBanks[6] = &prgData_[prgData_.size() - 0x4000];
    state_.CpuBanks[7] = &prgData_[prgData_.size() - 0x2000];

    prgMask_ = static_cast<uint32_t>(prgData_.size()) - 1;

    // initial state for mapper 5
    state_.PrgBank3 = prgMask_ & 0x01fe000;
}

void Cart::AddPrgRam(uint32_t size)
{
    prgRamMask_ = size - 1;
    localPrgRam_.resize(size);
    prgRamBanks_.push_back(&localPrgRam_[0]);
    state_.CpuBanks[3] = prgRamBanks_[0];
}

void Cart::AddPrgRam(uint8_t* data, uint32_t size)
{
    prgRamMask_ = size - 1;
    prgRamBanks_.push_back(data);

    if (mapper_ != MapperType::MMC6) // mapping done manually due to complex protection system
        state_.CpuBanks[3] = prgRamBanks_[0];
}

void Cart::SetChrRom(std::vector<uint8_t> chrData)
{
    chrData_ = std::move(chrData);

    state_.PpuBanks[0] = &chrData_[0];
    state_.PpuBanks[1] = &chrData_[0x0400];
    state_.PpuBanks[2] = &chrData_[0x0800];
    state_.PpuBanks[3] = &chrData_[0x0c00];
    state_.PpuBanks[4] = &chrData_[0x1000];
    state_.PpuBanks[5] = &chrData_[0x1400];
    state_.PpuBanks[6] = &chrData_[0x1800];
    state_.PpuBanks[7] = &chrData_[0x1c00];

    assert((chrData_.size() & (chrData_.size() - 1)) == 0);
    chrMask_ = static_cast<uint32_t>(chrData_.size()) - 1;
}

void Cart::SetChrRam(uint32_t size)
{
    chrData_.resize(size);

    state_.PpuBanks[0] = &chrData_[0];
    state_.PpuBanks[1] = &chrData_[0x0400];
    state_.PpuBanks[2] = &chrData_[0x0800];
    state_.PpuBanks[3] = &chrData_[0x0c00];
    state_.PpuBanks[4] = &chrData_[0x1000];
    state_.PpuBanks[5] = &chrData_[0x1400];
    state_.PpuBanks[6] = &chrData_[0x1800];
    state_.PpuBanks[7] = &chrData_[0x1c00];

    chrWriteable_ = true;
    assert((size & (size - 1)) == 0);
    chrMask_ = size - 1;
}

void Cart::SetMirrorMode(MirrorMode mirrorMode)
{
    state_.MirrorMode = mirrorMode;
    if (bus_)
    {
        UpdatePpuRamMap();
    }
}

void Cart::EnableBusConflicts(bool conflicts)
{
    busConflicts_ = conflicts;
}

void Cart::Attach(Bus* bus)
{
    bus_ = bus;
    UpdatePpuRamMap();
}

uint8_t Cart::CpuRead(uint16_t address)
{
    if (mapper_ == MapperType::MMC5)
    {
        if (state_.IrqPending && (address & 0xfffe) == 0xffffa)
        {
            state_.InFrame = false;
            state_.IrqPending = false;
            bus_->SetCartIrq(false);

            bus_->SyncPpu();
            state_.PpuInFrame = false;

            UpdateChrMapMMC5();
        }
    }

    auto bank = state_.CpuBanks[address >> 13];

    if (bank == nullptr)
    {
        if (mapper_ == MapperType::MMC5)
            return ReadMMC5(address);
        else if (mapper_ == MapperType::MMC6)
        {
            if (address < 0x8000)
            {
                auto protect = (address & 0200) != 0 ? state_.PrgRamProtect1 : state_.PrgRamProtect0;
                if ((protect & 6) != 0)
                    return prgRamBanks_[0][address & 0x3ff];
            }
        }

        return 0;
    }

    return bank[address & 0x1fff];
}

void Cart::CpuWrite(uint16_t address, uint8_t value)
{
    if (address < 0x8000)
    {
        if (mapper_ == MapperType::MMC5 && address < 0x6000)
        {
            WriteMMC5(address, value);
            return;
        }

        if (mapper_ == MapperType::MMC6)
        {
            auto protect = (address & 0200) != 0 ? state_.PrgRamProtect1 : state_.PrgRamProtect0;
            if ((protect & 5) != 0)
                prgRamBanks_[0][address & 0x3ff] = value;
            return;
        }

        if (mapper_ == MapperType::NINA001)
        {
            WriteNINA001(address, value);
            // continue to alse write to RAM
        }
        else if (mapper_ == MapperType::Caltron6in1)
        {
            WriteCaltron6in1Low(address);
            return;
        }

        if (state_.PrgRamProtect0)
            return;

        auto bank = state_.CpuBanks[address >> 13];
        if (bank)
            bank[address & 0x1fff] = value;
        return;
    }

    switch (mapper_)
    {
    case MapperType::MMC1:
        WriteMMC1(address, value);
        break;

    case MapperType::UxROM:
        WriteUxROM(address, value);
        break;

    case MapperType::CNROM:
        WriteCNROM(address, value);
        break;

    case MapperType::MMC3:
    case MapperType::MMC6:
    case MapperType::MCACC:
        WriteMMC3(address, value);
        break;

    case MapperType::MMC5:
        if (state_.CpuBankWritable[address >> 13])
            state_.CpuBanks[address >> 13][address & 0x1fff] = value;
        break;

    case MapperType::AxROM:
        WriteAxROM(address, value);
        break;

    case MapperType::MMC2:
        WriteMMC2(address, value);
        break;

    case MapperType::ColorDreams:
        WriteColorDreams(address, value);
        break;

    case MapperType::CPROM:
        WriteCPROM(address, value);
        break;

    case MapperType::BNROM:
        WriteBNROM(address, value);
        break;

    case MapperType::Caltron6in1:
        WriteCaltron6in1High(address, value);
        break;

    case MapperType::BF9093:
        if (address >= 0xc000)
            WriteUxROM(address, value);
        break;
    }
}

void Cart::CpuWrite2(uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
    if (address < 0x8000)
    {
        if (mapper_ == MapperType::MMC5 && address < 0x6000)
        {
            WriteMMC5(address, firstValue);
            bus_->TickCpuWrite();
            WriteMMC5(address, firstValue);
            return;
        }

        if (mapper_ == MapperType::NINA001)
        {
            WriteNINA001(address, firstValue);
            bus_->TickCpuWrite();
            WriteNINA001(address, secondValue);
            // continue to also write to RAM
        }
        else if (mapper_ == MapperType::Caltron6in1)
        {
            WriteCaltron6in1Low(address);
            bus_->TickCpuWrite();
            return;
        }
        else
        {
            // TODO: it's possible a bank switch happens underneath us
            bus_->TickCpuWrite();
        }

        if (mapper_ == MapperType::MMC6)
        {
            auto protect = (address & 0200) != 0 ? state_.PrgRamProtect1 : state_.PrgRamProtect0;
            if ((protect & 5) != 0)
                prgRamBanks_[0][address & 0x3ff] = secondValue;
            return;
        }
        else
        {
            if (state_.PrgRamProtect0)
                return;
        }

        auto bank = state_.CpuBanks[address >> 13];
        if (bank)
            bank[address & 0x1fff] = secondValue;
        return;
    }

    switch (mapper_)
    {
    case MapperType::MMC1:
        // The MMC1 takes the first value and ignores the second.
        WriteMMC1(address, firstValue);
        bus_->TickCpuWrite();
        break;

    case MapperType::UxROM:
        bus_->TickCpuWrite();
        WriteUxROM(address, secondValue);
        break;

    case MapperType::CNROM:
        WriteCNROM(address, firstValue);
        bus_->TickCpuWrite();
        WriteCNROM(address, secondValue);
        break;

    case MapperType::MMC3:
    case MapperType::MMC6:
    case MapperType::MCACC:
        WriteMMC3(address, firstValue);
        bus_->TickCpuWrite();
        WriteMMC3(address, secondValue);
        break;

    case MapperType::MMC5:
        bus_->TickCpuWrite();
        if (state_.CpuBankWritable[address >> 13])
            state_.CpuBanks[address >> 13][address & 0x1fff] = secondValue;
        break;

    case MapperType::AxROM:
        WriteAxROM(address, firstValue);
        bus_->TickCpuWrite();
        WriteAxROM(address, secondValue);
        break;

    case MapperType::MMC2:
        WriteMMC2(address, firstValue);
        bus_->TickCpuWrite();
        WriteMMC2(address, secondValue);
        break;

    case MapperType::ColorDreams:
        WriteColorDreams(address, firstValue);
        bus_->TickCpuWrite();
        WriteColorDreams(address, secondValue);
        break;

    case MapperType::CPROM:
        WriteCPROM(address, firstValue);
        bus_->TickCpuWrite();
        WriteCPROM(address, secondValue);
        break;

    case MapperType::BNROM:
        bus_->TickCpuWrite();
        WriteBNROM(address, secondValue);
        break;

    case MapperType::Caltron6in1:
        WriteCaltron6in1High(address, firstValue);
        bus_->TickCpuWrite();
        WriteCaltron6in1High(address, secondValue);

    case MapperType::BF9093:
        bus_->TickCpuWrite();
        if (address >= 0xc000)
            WriteUxROM(address, secondValue);
        break;

    default:
        bus_->TickCpuWrite();
        break;
    }
}

uint8_t Cart::PpuReadData(uint16_t address)
{
    auto bankIndex = address >> 10;

    if (mapper_ == MapperType::MMC5)
    {
        if (state_.ExtendedRamMode <= 1 && state_.PpuInFrame && !state_.InSprites)
        {
            if (state_.SplitEnabled)
            {
                auto isSplit = (state_.CurrentTile < state_.SplitTile) ^ state_.RightSplit;
                if (isSplit)
                {
                    // TODO: is the scanline counter the right thing to use here?
                    if (address >= 0x2000)
                    {
                        if ((address & 0x03ff) >= 0x3c0)
                        {
                            // attribute byte
                            auto attributeAddress = 0x03c0 + ((state_.SplitY & 0xe0) >> 2) | ((state_.CurrentTile & 0x1f) >> 2);
                            return state_.ExtendedRam[attributeAddress];
                        }
                        else
                        {
                            // nametable byte
                            auto tileAddress = ((state_.SplitY & 0xf8) << 2) | (state_.CurrentTile & 0x1f);
                            return state_.ExtendedRam[tileAddress];
                        }
                    }
                    else
                    {
                        return chrData_[state_.SplitBank | (address & 0xff8) | (state_.SplitY & 0x07)];
                    }
                }
            }

            if (state_.ExtendedRamMode == 1)
            {
                if (address < 0x2000)
                {
                    // pattern byte
                    auto chrAddress = state_.ExtendedPatternAddress | (address & 0x0fff);
                    return chrData_[chrAddress & (chrData_.size() - 1)];
                }
                else if ((address & 0x03ff) >= 0x03c0)
                {
                    return state_.ExtendedAttribute;
                }
                else
                {
                    auto exData = state_.ExtendedRam[address & 0x03ff];
                    state_.ExtendedAttribute = exData & 0xc0;
                    state_.ExtendedAttribute |= state_.ExtendedAttribute >> 2;
                    state_.ExtendedAttribute |= state_.ExtendedAttribute >> 4;

                    state_.ExtendedPatternAddress = state_.ChrBankHighBits | ((exData & 0x3f) << 12);
                }
            }
        }

        auto bank = state_.PpuBanks[bankIndex];
        if (bank == nullptr)
        {
            return (address & 0x03ff) >= 0x03c0 ?
                state_.PPuBankAttributeBytes[bankIndex & 0x03] :
                state_.PpuBankFillBytes[bankIndex & 0x03];
        }

        return bank[address & 0x03ff];
    }
    else if (mapper_ == MapperType::MMC2)
    {
        PpuReadMMC2(address);
    }

    auto bank = state_.PpuBanks[bankIndex];
    return bank[address & 0x03ff];
}

uint8_t Cart::PpuReadNametable(uint16_t address)
{
    auto bankIndex = address >> 10;

    if (mapper_ == MapperType::MMC5)
    {
        if (state_.ExtendedRamMode <= 1 && state_.PpuInFrame)
        {
            if (state_.SplitEnabled)
            {
                auto isSplit = (state_.CurrentTile < state_.SplitTile) ^ state_.RightSplit;
                if (isSplit)
                {
                    // it's possible for a nametable read to hit an attribute byte.
                    if ((address & 0x03ff) >= 0x3c0)
                    {
                        // attribute byte
                        auto attributeAddress = 0x03c0 | ((state_.SplitY & 0xe0) >> 2) | ((state_.CurrentTile & 0x1f) >> 2);
                        return state_.ExtendedRam[attributeAddress];
                    }
                    else
                    {
                        // nametable byte
                        auto tileAddress = ((state_.SplitY & 0xf8) << 2) | (state_.CurrentTile & 0x1f);
                        return state_.ExtendedRam[tileAddress];
                    }
                }
            }

            if (state_.ExtendedRamMode == 1)
            {
                // it's possible for a nametable read to hit an attribute byte.
                if ((address & 0x03ff) >= 0x03c0)
                {
                    return state_.ExtendedAttribute;
                }
                else
                {
                    auto exData = state_.ExtendedRam[address & 0x03ff];
                    state_.ExtendedAttribute = exData & 0xc0;
                    state_.ExtendedAttribute |= state_.ExtendedAttribute >> 2;
                    state_.ExtendedAttribute |= state_.ExtendedAttribute >> 4;

                    state_.ExtendedPatternAddress = state_.ChrBankHighBits | ((exData & 0x3f) << 12);
                }
            }
        }
        auto bank = state_.PpuBanks[bankIndex];
        if (bank == nullptr)
        {
            return (address & 0x03ff) >= 0x03c0 ?
                state_.PPuBankAttributeBytes[bankIndex & 0x03] :
                state_.PpuBankFillBytes[bankIndex & 0x03];
        }

        return bank[address & 0x03ff];
    }

    auto bank = state_.PpuBanks[bankIndex];
    return bank[address & 0x03ff];
}

uint8_t Cart::PpuReadAttributes(uint16_t address)
{
    auto bankIndex = address >> 10;

    if (mapper_ == MapperType::MMC5)
    {
        if (state_.ExtendedRamMode <= 1 && state_.PpuInFrame)
        {
            if (state_.SplitEnabled)
            {
                auto isSplit = (state_.CurrentTile < state_.SplitTile) ^ state_.RightSplit;
                if (isSplit)
                {
                    // attribute byte
                    auto attributeAddress = 0x03c0 | ((state_.SplitY & 0xe0) >> 2) | ((state_.CurrentTile & 0x1f) >> 2);
                    return state_.ExtendedRam[attributeAddress];
                }
            }

            if (state_.ExtendedRamMode == 1)
                return state_.ExtendedAttribute;
        }
        auto bank = state_.PpuBanks[bankIndex];
        if (bank == nullptr)
            return state_.PPuBankAttributeBytes[bankIndex & 0x03];

        return bank[address & 0x03ff];
    }

    auto bank = state_.PpuBanks[bankIndex];
    return bank[address & 0x03ff];
}

uint8_t Cart::PpuReadPatternLow(uint16_t address)
{
    auto bankIndex = address >> 10;

//  if (mapper_ == MapperType::MMC5)
    if (state_.ExtendedRamMode <= 1 && state_.PpuInFrame)
    {
        if (state_.SplitEnabled)
        {
            auto isSplit = (state_.CurrentTile < state_.SplitTile) ^ state_.RightSplit;
            if (isSplit)
            {
                return chrData_[state_.SplitBank | (address & 0xff8) | (state_.SplitY & 0x07)];
            }
        }

        if (state_.ExtendedRamMode == 1)
        {
            // pattern byte
            auto chrAddress = state_.ExtendedPatternAddress | (address & 0x0fff);
            return chrData_[chrAddress & (chrData_.size() - 1)];
        }
    }

    auto bank = state_.PpuBanks[bankIndex];
    return bank[address & 0x03ff];
}

uint8_t Cart::PpuReadPatternHigh(uint16_t address)
{
    auto bankIndex = address >> 10;

//  if (mapper_ == MapperType::MMC5)
    if (state_.ExtendedRamMode <= 1 && state_.PpuInFrame)
    {
        if (state_.SplitEnabled)
        {
            auto isSplit = (state_.CurrentTile < state_.SplitTile) ^ state_.RightSplit;
            if (isSplit)
            {
                return chrData_[state_.SplitBank | (address & 0xff8) | (state_.SplitY & 0x07)];
            }
        }

        if (state_.ExtendedRamMode == 1)
        {
            auto chrAddress = state_.ExtendedPatternAddress | (address & 0x0fff);
            return chrData_[chrAddress & (chrData_.size() - 1)];
        }
    }
    else if (mapper_ == MapperType::MMC2)
    {
        PpuReadMMC2(address);
    }

    auto bank = state_.PpuBanks[bankIndex];
    return bank[address & 0x03ff];
}

uint8_t Cart::PpuReadSpritePatternLow(uint16_t address)
{
    auto bankIndex = address >> 10;

    auto bank = state_.PpuBanks[bankIndex];
    return bank[address & 0x03ff];
}

uint8_t Cart::PpuReadSpritePatternHigh(uint16_t address)
{
    auto bankIndex = address >> 10;

    if (mapper_ == MapperType::MMC2)
    {
        PpuReadMMC2(address);
    }

    auto bank = state_.PpuBanks[bankIndex];
    return bank[address & 0x03ff];
}

uint16_t Cart::PpuReadPattern16(uint16_t address)
{
    auto bankIndex = address >> 10;

    //if (mapper_ == MapperType::MMC5)
    if (state_.ExtendedRamMode <= 1 && state_.PpuInFrame)
    {
        if (state_.SplitEnabled)
        {
            auto isSplit = (state_.CurrentTile < state_.SplitTile) ^ state_.RightSplit;
            if (isSplit)
            {
                auto romAddress = state_.SplitBank + (address & 0xfff);
                return (chrData_[romAddress | 8] << 8) | chrData_[romAddress];
            }
        }

        if (state_.ExtendedRamMode == 1)
        {
            assert(address < 0x2000);

            // pattern byte
            auto chrRamAddress = state_.ExtendedPatternAddress | (address & 0x0fff);
            chrRamAddress &= (chrData_.size() - 1);
            return (chrData_[chrRamAddress | 8] << 8) | chrData_[chrRamAddress];
        }
    }
    else if (mapper_ == MapperType::MMC2)
    {
        PpuReadMMC2(address | 8);
    }

    auto bank = state_.PpuBanks[bankIndex];
    auto bankAddress = address & 0x03ff;
    return (bank[bankAddress | 8] << 8) | bank[bankAddress];
}

void Cart::PpuWrite(uint16_t address, uint8_t value)
{
    //assert(((address & 0x1000) != 0) == chrA12_);

    if (address >= 0x2000 || chrWriteable_)
    {
        auto bank = state_.PpuBanks[address >> 10];
        if (bank != nullptr)
            bank[address & 0x03ff] = value;
    }
}

ChrA12Sensitivity Cart::ChrA12Sensitivity() const
{
    return state_.ChrA12Sensitivity;
}

void Cart::ChrA12Rising()
{
    state_.ChrA12 = true;

    if (mapper_ == MapperType::MMC1)
    {
        if (state_.ChrA12Sensitivity == ChrA12Sensitivity::AllEdges)
        {
            UpdatePrgMapMMC1();

            if (state_.CpuBanks[3])
                state_.CpuBanks[3] = prgRamBanks_[state_.PrgRamBank1];
        }
    }
    else // MMC3, MMC6
    {
        if (state_.ChrA12Sensitivity == ChrA12Sensitivity::RisingEdgeSmoothed)
        {
            // MMC3 takes a smoothed signal, and clocks on the rising edge of A12
            ClockScanlineCounter();
        }
    }
}

void Cart::ChrA12Falling()
{
    state_.ChrA12 = false;

    if (mapper_ == MapperType::MMC1)
    {
        if (state_.ChrA12Sensitivity == ChrA12Sensitivity::AllEdges)
        {
            if (state_.CpuBanks[3])
                state_.CpuBanks[3] = prgRamBanks_[state_.PrgRamBank0];
        }
    }
    else // if (mapper_ == MapperType::MCACC)
    {
        if (state_.ChrA12Sensitivity == ChrA12Sensitivity::FallingEdgeDivided)
        {
            // MC-ACC clocks on every falling edge of A12, and triggers the scanline counter every 8 times.
            state_.ChrA12PulseCounter--;
            if (state_.ChrA12PulseCounter == 0)
            {
#ifdef DIAGNOSTIC
                bus_->MarkDiagnostic(0xff0000ff);
#endif 

                ClockScanlineCounter();
                state_.ChrA12PulseCounter = 8;
            }
        }
    }
}

uint32_t Cart::A12PulsesUntilSync()
{
    return mapper_ == MapperType::MCACC ? state_.ChrA12PulseCounter : 0;
}

bool Cart::HasScanlineCounter() const
{
    return mapper_ == MapperType::MMC5;
}

void Cart::ScanlineCounterBeginScanline()
{
#ifdef DIAGNOSTIC
    bus_->MarkDiagnostic(0xff00ffff);
#endif

    if (state_.InFrame)
    {
        state_.ScanlineCounter++;
        if (state_.ScanlineCounter == state_.InterruptScanline)
        {
            state_.IrqPending = true;
            if (state_.IrqEnabled)
                bus_->SetCartIrq(true);
        }
    }
    else
    {
        state_.InFrame = true;
        state_.ScanlineCounter = 0;
        state_.IrqPending = false;
        bus_->SetCartIrq(false);
    }
}

void Cart::ScanlineCounterEndFrame()
{
    state_.InFrame = false;
    state_.IrqPending = false;

    if (state_.RenderingEnabled && state_.LargeSprites && mapper_ == MapperType::MMC5)
    {
        bus_->SyncPpu();
        state_.PpuInFrame = false;
        UpdateChrMapMMC5();
    }
    else
    {
        state_.PpuInFrame = false;
    }

    state_.SplitY = state_.SplitScroll;

    bus_->SetCartIrq(false);
}

void Cart::TileSplitBeginScanline()
{
    state_.PpuInFrame = true;

    if (mapper_ == MapperType::MMC5)
        UpdateChrMapMMC5();
}

void Cart::TileSplitBeginTile(uint32_t tileIndex)
{
    // If the first tile in a scanline is in attribute space, it's possible for the MMC5 scanline counter to get one cycle out of sync.
    // However, I don't think this has any observable impact, so we can trust that it matches the PPU counter.
    // TODO: check this is actually true.
    state_.CurrentTile = tileIndex;
}

void Cart::TileSplitBeginSprites()
{
    state_.InSprites = true;
    if (mapper_ == MapperType::MMC5 && state_.PpuInFrame)
        UpdateChrMapMMC5();
}

void Cart::TileSplitEndSprites()
{
    state_.InSprites = false;
    if (mapper_ == MapperType::MMC5 && state_.PpuInFrame)
    {
        UpdateChrMapMMC5();

        state_.SplitY++;
        if (state_.SplitY == 240)
        {
            state_.SplitY = 0;
        }
    }
}

void Cart::InterceptWritePpuCtrl(bool largeSprites)
{
    state_.LargeSprites = largeSprites;
}

void Cart::InterceptWritePpuMask(bool renderingEnabled)
{
    state_.RenderingEnabled = renderingEnabled;
    if (mapper_ == MapperType::MMC5)
        UpdateChrMapMMC5();
}

bool Cart::UsesMMC5Audio() const
{
    //return mapper_ == 5;
    return false; // only available on Famicom.
}

void Cart::CaptureState(CartState* state) const
{
    state->Core = state_;

    if (prgRamBanks_.size() > 0)
        std::copy(prgRamBanks_[0], prgRamBanks_[0] + prgRamMask_ + 1, begin(state->PrgRamBank1));

    if (prgRamBanks_.size() > 1)
        std::copy(prgRamBanks_[1], prgRamBanks_[1] + prgRamMask_ + 1, begin(state->PrgRamBank2));

    assert(prgRamBanks_.size() <= 2);

    if (chrWriteable_)
        std::copy(begin(chrData_), end(chrData_), begin(state->ChrRam));
}

void Cart::RestoreState(const CartState& state)
{
    state_ = state.Core;

    if (prgRamBanks_.size() > 0)
        std::copy(begin(state.PrgRamBank1), end(state.PrgRamBank1), prgRamBanks_[0]);

    if (prgRamBanks_.size() > 1)
        std::copy(begin(state.PrgRamBank2), end(state.PrgRamBank2), prgRamBanks_[1]);

    if (chrWriteable_)
        std::copy(begin(state.ChrRam), end(state.ChrRam), begin(chrData_));

}

void Cart::WriteMMC1(uint16_t address, uint8_t value)
{
    if ((value & 0x80) == 0x80)
    {
        state_.MapperShiftCount = 0;
        state_.MapperShift = 0;

        // when the shift register is reset, the control bit is or'd with 0x0C
        state_.PrgMode = 3;
        UpdatePrgMapMMC1();
        return;
    }

    state_.MapperShift |= (value & 1) << state_.MapperShiftCount;
    state_.MapperShiftCount++;

    if (state_.MapperShiftCount == 5)
    {
        WriteMMC1Register(address, state_.MapperShift);

        state_.MapperShiftCount = 0;
        state_.MapperShift = 0;
    }
}

void Cart::WriteMMC1Register(uint16_t address, uint8_t value)
{
    switch (address >> 13)
    {
    case 4:
        // control

        // chr mode
        state_.ChrMode = (value >> 4) & 0x01;
        UpdateChrMapMMC1();

        // prg mode
        state_.PrgMode = (value >> 2) & 0x03;
        UpdatePrgMapMMC1();

        // mirroring mode
        state_.MirrorMode = static_cast<MirrorMode>(value & 0x03);
        UpdatePpuRamMap();
        break;

    case 5:
    {
        // CHR bank 0
        state_.ChrBank0 = value << 12;
        UpdateChrMapMMC1();

        auto sensitivityBefore = state_.ChrA12Sensitivity;

        state_.ChrA12Sensitivity = ChrA12Sensitivity::None;

        // TODO: SNROM PRG RAM disable line
        if (prgData_.size() == 0x80000)
        {
            state_.PrgPlane0 = ((state_.ChrBank0 << 2) & 0x40000);
            if (state_.PrgPlane0 != state_.PrgPlane1)
                state_.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (!state_.ChrA12)
                UpdatePrgMapMMC1();
        }

        if (prgRamBanks_.size() > 1)
        {
            if (prgRamBanks_.size() > 2)
                state_.PrgRamBank0 = (value >> 2) & 0x03;
            else
                state_.PrgRamBank0 = (value >> 1) & 0x01;

            if (state_.PrgRamBank0 != state_.PrgRamBank1)
                state_.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (!state_.ChrA12 && state_.CpuBanks[3])
                state_.CpuBanks[3] = prgRamBanks_[state_.PrgRamBank0];
        }

        if (state_.ChrA12Sensitivity != sensitivityBefore)
            bus_->UpdateA12Sensitivity();
        break;
    }

    case 6:
    {
        // CHR bank 1
        state_.ChrBank1 = value << 12;
        UpdateChrMapMMC1();

        auto sensitivityBefore = state_.ChrA12Sensitivity;
        state_.ChrA12Sensitivity = ChrA12Sensitivity::None;

        // TODO: SNROM PRG RAM disable line
        if (prgData_.size() == 0x80000)
        {
            state_.PrgPlane1 = ((state_.ChrBank1 << 2) & 0x40000);

            if (state_.PrgPlane0 != state_.PrgPlane1)
                state_.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (state_.ChrA12)
                UpdatePrgMapMMC1();
        }

        // TODO: SZROM maps this differently.
        if (prgRamBanks_.size() > 1)
        {
            if (prgRamBanks_.size() > 2)
                state_.PrgRamBank1 = (value >> 2) & 0x03;
            else
                state_.PrgRamBank1 = (value >> 1) & 0x01;

            if (state_.PrgRamBank0 != state_.PrgRamBank1)
                state_.ChrA12Sensitivity = ChrA12Sensitivity::AllEdges;

            // TODO: we need to get the current A12 state from the PPU.
            if (state_.ChrA12 && state_.CpuBanks[3])
                state_.CpuBanks[3] = prgRamBanks_[state_.PrgRamBank1];
        }

        if (state_.ChrA12Sensitivity != sensitivityBefore)
            bus_->UpdateA12Sensitivity();

        break;
    }

    case 7:
        // PRG bank

        // enable/disable PRG RAM
        if (value & 0x10 || prgRamBanks_.size() == 0)
            state_.CpuBanks[3] = nullptr;
        else
            state_.CpuBanks[3] = state_.ChrA12 ? prgRamBanks_[state_.PrgRamBank1] : prgRamBanks_[state_.PrgRamBank0];

        state_.PrgBank0 = ((value & 0x0f) << 14 & prgMask_);
        UpdatePrgMapMMC1();
        break;
    }
}

void Cart::UpdateChrMapMMC1()
{
    bus_->SyncPpu();

    switch (state_.ChrMode)
    {
    case 0:
    {
        auto chrBank = static_cast<size_t>(state_.ChrBank0) & 0x1e000;
        auto base = &chrData_[chrBank];
        state_.PpuBanks[0] = base;
        state_.PpuBanks[1] = base + 0x0400;
        state_.PpuBanks[2] = base + 0x0800;
        state_.PpuBanks[3] = base + 0x0c00;
        state_.PpuBanks[4] = base + 0x1000;
        state_.PpuBanks[5] = base + 0x1400;
        state_.PpuBanks[6] = base + 0x1800;
        state_.PpuBanks[7] = base + 0x1c00;
        break;
    }

    case 1:
    {
        auto base0 = &chrData_[state_.ChrBank0 & (chrData_.size() - 1)];
        state_.PpuBanks[0] = base0;
        state_.PpuBanks[1] = base0 + 0x0400;
        state_.PpuBanks[2] = base0 + 0x0800;
        state_.PpuBanks[3] = base0 + 0x0c00;

        auto base1 = &chrData_[state_.ChrBank1 & (chrData_.size() - 1)];
        state_.PpuBanks[4] = base1;
        state_.PpuBanks[5] = base1 + 0x0400;
        state_.PpuBanks[6] = base1 + 0x0800;
        state_.PpuBanks[7] = base1 + 0x0c00;
        break;
    }
    }
}

void Cart::UpdatePrgMapMMC1()
{
    auto prgPlane = state_.ChrA12 ? state_.PrgPlane1 : state_.PrgPlane0;

    switch (state_.PrgMode)
    {
    case 0:
    case 1:
    {
        auto base = &prgData_[prgPlane | (state_.PrgBank0 & 0xffff8000)];
        state_.CpuBanks[4] = base;
        state_.CpuBanks[5] = base + 0x2000;
        state_.CpuBanks[6] = base + 0x4000;
        state_.CpuBanks[7] = base + 0x6000;
        break;
    }

    case 2:
    {
        auto base = &prgData_[prgPlane | state_.PrgBank0];
        state_.CpuBanks[4] = &prgData_[prgPlane];
        state_.CpuBanks[5] = &prgData_[prgPlane | 0x2000];
        state_.CpuBanks[6] = base;
        state_.CpuBanks[7] = base + 0x2000;
        break;
    }

    case 3:
    {
        auto base = &prgData_[prgPlane | state_.PrgBank0];
        state_.CpuBanks[4] = base;
        state_.CpuBanks[5] = base + 0x2000;
        state_.CpuBanks[6] = &prgData_[prgPlane | (prgData_.size() - 0x4000)];
        state_.CpuBanks[7] = &prgData_[prgPlane | (prgData_.size() - 0x2000)];
    }
    }
}

void Cart::WriteUxROM(uint16_t address, uint8_t value)
{
    if (busConflicts_)
    {
        auto bank = state_.CpuBanks[address >> 13];
        value &= bank[address & 0x1fff];
    }

    auto bankAddress = (value << 14) & prgMask_;
    auto base = &prgData_[bankAddress];
    state_.CpuBanks[4] = base;
    state_.CpuBanks[5] = base + 0x2000;
}

void Cart::WriteCNROM(uint16_t address, uint8_t value)
{
    if (busConflicts_)
    {
        auto bank = state_.CpuBanks[address >> 13];
        value &= bank[address & 0x1fff];
    }

    // the actual board only takes 2 bits from the value for bank switching
    auto bankAddress = (value << 13) & chrMask_;

    auto base = &chrData_[bankAddress];

    if (base != state_.PpuBanks[0])
    {
        bus_->SyncPpu();
        state_.PpuBanks[0] = base;
        state_.PpuBanks[1] = base + 0x0400;
        state_.PpuBanks[2] = base + 0x0800;
        state_.PpuBanks[3] = base + 0x0c00;
        state_.PpuBanks[4] = base + 0x1000;
        state_.PpuBanks[5] = base + 0x1400;
        state_.PpuBanks[6] = base + 0x1800;
        state_.PpuBanks[7] = base + 0x1c00;
    }
}

void Cart::WriteMMC3(uint16_t address, uint8_t value)
{
    if (address < 0xa000)
    {
        if ((address & 1) == 0)
        {
            state_.PrgBank0 = value & 0x07;
            SetPrgModeMMC3((value >> 6) & 1);
            SetChrModeMMC3((value >> 7) & 1);

            if (mapper_ == MapperType::MMC6)
            {
                // PRG RAM enable
                // use bit 3 of the protect flag to signal this 
                if ((value & 0x20) != 0)
                {
                    state_.PrgRamProtect0 |= 4;
                    state_.PrgRamProtect1 |= 4;
                }
                else
                {
                    state_.PrgRamProtect0 &= ~4;
                    state_.PrgRamProtect1 &= ~4;
                }
            }
        }
        else
        {
            SetBankMMC3(value);
        }
    }
    else if (address < 0xc000)
    {
        if ((address & 1) == 0)
        {
            if (state_.MirrorMode != MirrorMode::FourScreen)
            {
                auto newMirrorMode = value & 1 ? MirrorMode::Horizontal : MirrorMode::Vertical;
                if (state_.MirrorMode != newMirrorMode)
                {
                    state_.MirrorMode = newMirrorMode;
                    UpdatePpuRamMap();
                }
            }
        }
        else
        {
            if (mapper_ == MapperType::MMC6)
            {
                state_.PrgRamProtect0 &= ~3;
                state_.PrgRamProtect0 |= (value >> 4 & 3);

                state_.PrgRamProtect1 &= ~3;
                state_.PrgRamProtect1 |= (value >> 6 & 3);
            }
            else
            {
                auto ramEnabled = (value & 0x80) != 0;
                state_.PrgRamProtect0 = (value & 0x40) != 0;
                state_.CpuBanks[3] = ramEnabled && prgRamBanks_.size() ? prgRamBanks_[0] : nullptr;
            }
        }
    }
    else if (address < 0xe000)
    {
        if ((address & 1) == 0)
        {
            state_.ReloadValue = value;
        }
        else
        {
            state_.ReloadCounter = true;

            // MC-ACC:
            // "writing to 0xC001 resets the pulse counter"
            state_.ChrA12PulseCounter = 1;

#ifdef DIAGNOSTIC
            bus_->MarkDiagnostic(0xff444444);
#endif
        }

        auto sensitivityBefore = state_.ChrA12Sensitivity;
        if (state_.ScanlineCounter > 0 || (state_.ReloadValue > 0) || state_.IrqEnabled)
            state_.ChrA12Sensitivity =
                mapper_ == MapperType::MCACC
                    ? ChrA12Sensitivity::FallingEdgeDivided
                    : ChrA12Sensitivity::RisingEdgeSmoothed;
        else
            state_.ChrA12Sensitivity = ChrA12Sensitivity::None;

        if (state_.ChrA12Sensitivity != sensitivityBefore)
            bus_->UpdateA12Sensitivity();

    }
    else
    {
        if ((address & 1) == 0)
        {
            state_.IrqEnabled = false;
            bus_->SetCartIrq(false);
        }
        else
        {
            state_.IrqEnabled = true;
        }

        auto sensitivityBefore = state_.ChrA12Sensitivity;
        if (state_.ScanlineCounter > 0 || (state_.ReloadValue > 0) || state_.IrqEnabled)
            state_.ChrA12Sensitivity =
            mapper_ == MapperType::MCACC
            ? ChrA12Sensitivity::FallingEdgeDivided
            : ChrA12Sensitivity::RisingEdgeSmoothed;
        else
            state_.ChrA12Sensitivity = ChrA12Sensitivity::None;

        if (state_.ChrA12Sensitivity != sensitivityBefore)
            bus_->UpdateA12Sensitivity();
    }
}

void Cart::SetPrgModeMMC3(uint8_t mode)
{
    if (mode != state_.PrgMode)
    {
        std::swap(state_.CpuBanks[4], state_.CpuBanks[6]);

        state_.PrgMode = mode;
    }
}

void Cart::SetChrModeMMC3(uint8_t mode)
{
    if (mode != state_.ChrMode)
    {
        std::swap(state_.PpuBanks[0], state_.PpuBanks[4]);
        std::swap(state_.PpuBanks[1], state_.PpuBanks[5]);
        std::swap(state_.PpuBanks[2], state_.PpuBanks[6]);
        std::swap(state_.PpuBanks[3], state_.PpuBanks[7]);

        state_.ChrMode = mode;
    }
}

void Cart::SetBankMMC3(uint32_t bank)
{
    switch (state_.PrgBank0)
    {
    case 0:
        bus_->SyncPpu();
        SetChrBank2k(state_.ChrMode ? 4 : 0, bank);
        break;

    case 1:
        bus_->SyncPpu();
        SetChrBank2k(state_.ChrMode ? 6 : 2, bank);
        break;

    case 2:
        bus_->SyncPpu();
        SetChrBank1k(state_.ChrMode ? 0 : 4, bank);
        break;

    case 3:
        bus_->SyncPpu();
        SetChrBank1k(state_.ChrMode ? 1 : 5, bank);
        break;

    case 4:
        bus_->SyncPpu();
        SetChrBank1k(state_.ChrMode ? 2 : 6, bank);
        break;

    case 5:
        bus_->SyncPpu();
        SetChrBank1k(state_.ChrMode ? 3 : 7, bank);
        break;

    case 6:
        state_.CpuBanks[state_.PrgMode ? 6 : 4] = &prgData_[(bank << 13) & prgMask_];
        break;

    case 7:
        state_.CpuBanks[5] = &prgData_[(bank << 13) & prgMask_];
        break;
    }
}

void Cart::ClockScanlineCounter()
{
    if (state_.ScanlineCounter == 0 || state_.ReloadCounter)
    {
        state_.ScanlineCounter = state_.ReloadValue;
        state_.ReloadCounter = false;

        if (state_.ScanlineCounter == 0 && !state_.IrqEnabled)
        {
            state_.ChrA12Sensitivity = ChrA12Sensitivity::None;
            bus_->UpdateA12Sensitivity();
        }
    }
    else
    {
        state_.ScanlineCounter--;
    }

    if (state_.ScanlineCounter == 0 && state_.IrqEnabled)
    {
        bus_->SetCartIrq(true);
    }
}

uint8_t Cart::ReadMMC5(uint16_t address)
{
    if (address < 0x6000)
    {
        if (address >= 0x5c00)
        {
            if (state_.ExtendedRamMode >= 2)
            {
                return state_.ExtendedRam[address - 0x5c00];
            }

            return 0;
        }

        switch (address)
        {
        case 0x5015:
            return 0;

        case 0x5204:
        {
            uint8_t data = 0;

            if (state_.IrqPending)
            {
                data |= 0x80;
                state_.IrqPending = false;
                bus_->SetCartIrq(false);
            }

            if (state_.InFrame)
                data |= 0x40;

            return data;
        }

        case 0x5205:
            return (state_.MulitplierArg0 * state_.MulitplierArg1);

        case 0x5206:
            return (state_.MulitplierArg0 * state_.MulitplierArg1) >> 8;

        default:
            assert(false);
        }
    }

    return 0;
}

void Cart::WriteMMC5(uint16_t address, uint8_t value)
{
    if (address < 0x5020)
    {
        bus_->WriteMMC5Audio(address, value);
        return;
    }

    if (address >= 0x5c00)
    {
        if (state_.ExtendedRamMode > 2)
        {
            // TODO: if PPU is not rendering, this should write 0
            return;
        }

        state_.ExtendedRam[address - 0x5c00] = value;
        return;
    }

    switch (address)
    {
    case 0x5100:
        state_.PrgMode = value & 0x03;
        UpdatePrgMapMMC5();
        break;

    case 0x5101:
        state_.ChrMode = value & 0x03;
        UpdateChrMapMMC5();
        break;

    case 0x5102:
        if ((value & 0x03) == 0x02)
            state_.PrgRamProtect0 &= 0xfd;
        else
            state_.PrgRamProtect0 |= 0x02;
        UpdatePrgMapMMC5();
        break;

    case 0x5103:
        if ((value & 0x03) == 0x01)
            state_.PrgRamProtect0 &= 0xfe;
        else
            state_.PrgRamProtect0 |= 0x01;
        UpdatePrgMapMMC5();
        break;

    case 0x5104:
        bus_->SyncPpu();
        state_.ExtendedRamMode = value & 0x03;
        UpdateNametableMapMMC5();
        break;

    case 0x5105:
        bus_->SyncPpu();
        state_.NametableMode0 = value & 0x03;
        state_.NametableMode1 = (value >> 2) & 0x03;
        state_.NametableMode2 = (value >> 4) & 0x03;
        state_.NametableMode3 = (value >> 6) & 0x03;
        UpdateNametableMapMMC5();
        break;

    case 0x5106:
        // TODO: we could indirect this to avoid having to update the map.
        bus_->SyncPpu();
        state_.ChrFillValue = value;
        UpdateNametableMapMMC5();
        break;

    case 0x5107:
        bus_->SyncPpu();

        // copy the 2 bits 4 times to fill the attribute byte.
        value &= 0x03;
        value |= value << 2;
        value |= value << 4;
        state_.ChrFillAttributes = value;
        UpdateNametableMapMMC5();
        break;

    case 0x5113:
    {
        auto bank = prgRamBanks_[(value & 7) >> 2];
        state_.CpuBanks[3] = bank ? &bank[(value << 13) & prgRamMask_] : nullptr;
        break;
    }

    case 0x5114:
        state_.PrgBank0Ram = (value & 0x80) == 0;
        state_.PrgBank0 = state_.PrgBank0Ram
            ? (value & 7)
            : (value << 13) & prgMask_;
        UpdatePrgMapMMC5();
        break;

    case 0x5115:
        state_.PrgBank1Ram = (value & 0x80) == 0;
        state_.PrgBank1 = state_.PrgBank1Ram
            ? (value & 7)
            : (value << 13) & prgMask_;
        UpdatePrgMapMMC5();
        break;

    case 0x5116:
        state_.PrgBank2Ram = (value & 0x80) == 0;
        state_.PrgBank2 = state_.PrgBank2Ram
            ? (value & 7)
            : (value << 13) & prgMask_;
        UpdatePrgMapMMC5();
        break;

    case 0x5117:
        state_.PrgBank3 = (value << 13) & prgMask_;
        UpdatePrgMapMMC5();
        break;

    case 0x5120:
        bus_->SyncPpu();
        state_.ChrBank0 = value;
        state_.UseSecondaryChrForData = false;
        UpdateChrMapMMC5();
        break;

    case 0x5121:
        bus_->SyncPpu();
        state_.ChrBank1 = value;
        state_.UseSecondaryChrForData = false;
        UpdateChrMapMMC5();
        break;

    case 0x5122:
        bus_->SyncPpu();
        state_.ChrBank2 = value;
        state_.UseSecondaryChrForData = false;
        UpdateChrMapMMC5();
        break;

    case 0x5123:
        bus_->SyncPpu();
        state_.ChrBank3 = value;
        state_.UseSecondaryChrForData = false;
        UpdateChrMapMMC5();
        break;

    case 0x5124:
        bus_->SyncPpu();
        state_.ChrBank4 = value;
        state_.UseSecondaryChrForData = false;
        UpdateChrMapMMC5();
        break;

    case 0x5125:
        bus_->SyncPpu();
        state_.ChrBank5 = value;
        state_.UseSecondaryChrForData = false;
        UpdateChrMapMMC5();
        break;

    case 0x5126:
        bus_->SyncPpu();
        state_.ChrBank6 = value;
        state_.UseSecondaryChrForData = false;
        UpdateChrMapMMC5();
        break;

    case 0x5127:
        bus_->SyncPpu();
        state_.ChrBank7 = value;
        state_.UseSecondaryChrForData = false;
        UpdateChrMapMMC5();
        break;

    case 0x5128:
        bus_->SyncPpu();
        state_.SecondaryChrBank0 = value;
        state_.UseSecondaryChrForData = true;
        UpdateChrMapMMC5();
        break;

    case 0x5129:
        bus_->SyncPpu();
        state_.SecondaryChrBank1 = value;
        state_.UseSecondaryChrForData = true;
        UpdateChrMapMMC5();
        break;

    case 0x512A:
        bus_->SyncPpu();
        state_.SecondaryChrBank2 = value;
        state_.UseSecondaryChrForData = true;
        UpdateChrMapMMC5();
        break;

    case 0x512B:
        bus_->SyncPpu();
        state_.SecondaryChrBank3 = value;
        state_.UseSecondaryChrForData = true;
        UpdateChrMapMMC5();
        break;

    case 0x5130:
        bus_->SyncPpu();
        state_.ChrBankHighBits = (value & 3) << 14;
        break;

    case 0x5200:
        bus_->SyncPpu();
        state_.SplitEnabled = (value & 0x80) != 0;
        state_.RightSplit = (value & 0x40) != 0;
        state_.SplitTile = value & 0x1f;
        UpdateNametableMapMMC5();
        break;

    case 0x5201:
        bus_->SyncPpu();
        state_.SplitScroll = value;
        break;

    case 0x5202:
        bus_->SyncPpu();
        state_.SplitBank = (value << 12) & chrMask_;
        UpdateChrMapMMC5();
        break;

    case 0x5203:
        state_.InterruptScanline = value;
        break;

    case 0x5204:
        state_.IrqEnabled = (value & 0x80) != 0;
        if (state_.IrqPending)
            bus_->SetCartIrq(state_.IrqEnabled);
        break;

    case 0x5205:
        state_.MulitplierArg0 = value;
        break;

    case 0x5206:
        state_.MulitplierArg1 = value;
        break;

    default:
        assert(false);
    }

    return;
}

void Cart::UpdatePrgMapMMC5()
{
    switch (state_.PrgMode)
    {
    case 0:
    {
        auto base = &prgData_[(state_.PrgBank3 & 0xffff8000)];
        state_.CpuBanks[4] = base;
        state_.CpuBanks[5] = base + 0x2000;
        state_.CpuBanks[6] = base + 0x4000;
        state_.CpuBanks[7] = base + 0x6000;

        state_.CpuBankWritable[4] = false;
        state_.CpuBankWritable[5] = false;
        state_.CpuBankWritable[6] = false;
        state_.CpuBankWritable[7] = false;
        break;
    }

    case 1:
    {
        if (state_.PrgBank1Ram)
        {
            auto prgRamIndex = state_.PrgBank1 >> 2;
            auto bank = prgRamIndex < prgRamBanks_.size() ? prgRamBanks_[prgRamIndex] : nullptr;
            if (bank)
            {
                // TODO: what happens if the bank doesn't align with a 4k boundary?
                state_.CpuBanks[4] = bank + (((state_.PrgBank1 & 0x03 & ~1) << 13) & prgRamMask_);
                state_.CpuBanks[5] = bank + (((state_.PrgBank1 & 0x03 | 1) << 13) & prgRamMask_);

                state_.CpuBankWritable[4] = state_.PrgRamProtect0 == 0;
                state_.CpuBankWritable[5] = state_.PrgRamProtect0 == 0;
            }
            else
            {
                state_.CpuBanks[4] = nullptr;
                state_.CpuBanks[5] = nullptr;

                state_.CpuBankWritable[4] = false;
                state_.CpuBankWritable[5] = false;
            }
        }
        else
        {
            auto baseLow = &prgData_[(state_.PrgBank1 & 0xffffc000)];
            state_.CpuBanks[4] = baseLow;
            state_.CpuBanks[5] = baseLow + 0x2000;

            state_.CpuBankWritable[4] = false;
            state_.CpuBankWritable[5] = false;
        }

        auto baseHigh = &prgData_[(state_.PrgBank3 & 0xffffc000)];
        state_.CpuBanks[6] = baseHigh;
        state_.CpuBanks[7] = baseHigh + 0x2000;

        state_.CpuBankWritable[6] = false;
        state_.CpuBankWritable[7] = false;
        break;
    }

    case 2:
    {
        if (state_.PrgBank1Ram)
        {
            auto prgRamIndex = state_.PrgBank1 >> 2;
            auto bank = prgRamIndex < prgRamBanks_.size() ? prgRamBanks_[prgRamIndex] : nullptr;
            if (bank)
            {
                // TODO: what happens if the bank doesn't align with a 4k boundary?
                state_.CpuBanks[4] = bank + (((state_.PrgBank1 & 0x03 & ~1) << 13) & prgRamMask_);
                state_.CpuBanks[5] = bank + (((state_.PrgBank1 & 0x03 | 1) << 13) & prgRamMask_);

                state_.CpuBankWritable[4] = state_.PrgRamProtect0 == 0;
                state_.CpuBankWritable[5] = state_.PrgRamProtect0 == 0;
            }
            else
            {
                state_.CpuBanks[4] = nullptr;
                state_.CpuBanks[5] = nullptr;

                state_.CpuBankWritable[4] = false;
                state_.CpuBankWritable[5] = false;
            }
        }
        else
        {
            auto baseLow = &prgData_[(state_.PrgBank1 & 0xffffc000)];
            state_.CpuBanks[4] = baseLow;
            state_.CpuBanks[5] = baseLow + 0x2000;

            state_.CpuBankWritable[4] = false;
            state_.CpuBankWritable[5] = false;
        }

        MapPrgBankMMC5(state_.PrgBank2Ram, state_.PrgBank2, &state_.CpuBanks[6], &state_.CpuBankWritable[6]);


        state_.CpuBanks[7] = &prgData_[(state_.PrgBank3)];
        state_.CpuBankWritable[7] = false;
        break;
    }

    case 3:
    {
        MapPrgBankMMC5(state_.PrgBank0Ram, state_.PrgBank0, &state_.CpuBanks[4], &state_.CpuBankWritable[4]);
        MapPrgBankMMC5(state_.PrgBank1Ram, state_.PrgBank1, &state_.CpuBanks[5], &state_.CpuBankWritable[5]);
        MapPrgBankMMC5(state_.PrgBank2Ram, state_.PrgBank2, &state_.CpuBanks[6], &state_.CpuBankWritable[6]);

        state_.CpuBanks[7] = &prgData_[(state_.PrgBank3)];
        state_.CpuBankWritable[7] = false;
    }
    }
}

void Cart::MapPrgBankMMC5(bool isRam, int32_t index, uint8_t** bank, bool* writable)
{
    if (isRam)
    {
        auto prgRamIndex = index >> 2;
        auto ramBank = prgRamIndex < prgRamBanks_.size() ? prgRamBanks_[prgRamIndex] : nullptr;
        if (ramBank)
        {
            *bank = ramBank + (((index & 0x03) << 13) & prgRamMask_);
            *writable = state_.PrgRamProtect0 == 0;
        }
        else
        {
            *bank = nullptr;
            *writable = false;
        }
    }
    else
    {
        *bank = &prgData_[index];
        *writable = false;
    }
}

void Cart::UpdateChrMapMMC5()
{
    bool useSecondary = false;
    if (state_.LargeSprites)
    {
        if (state_.RenderingEnabled && state_.PpuInFrame)
        {
            useSecondary = !state_.InSprites;
        }
        else
            useSecondary = state_.UseSecondaryChrForData;
    }

    switch (state_.ChrMode)
    {
    case 0:
    {
        uint8_t* base;
        if (useSecondary)
            base = &chrData_[(state_.SecondaryChrBank3 << 13) & chrMask_];
        else
            base = &chrData_[(state_.ChrBank7 << 13) & chrMask_];

        state_.PpuBanks[0] = base;
        state_.PpuBanks[1] = base + 0x0400;
        state_.PpuBanks[2] = base + 0x0800;
        state_.PpuBanks[3] = base + 0x0c00;
        state_.PpuBanks[4] = base + 0x1000;
        state_.PpuBanks[5] = base + 0x1400;
        state_.PpuBanks[6] = base + 0x1800;
        state_.PpuBanks[7] = base + 0x1c00;
        break;
    }

    case 1:
    {
        uint8_t* baseLow;
        uint8_t* baseHigh;
        if (useSecondary)
        {
            baseLow = baseHigh = &chrData_[(state_.SecondaryChrBank3 << 12) & chrMask_];
        }
        else
        {
            baseLow = &chrData_[(state_.ChrBank3 << 12) & chrMask_];
            baseHigh = &chrData_[(state_.ChrBank7 << 12) & chrMask_];
        }

        state_.PpuBanks[0] = baseLow;
        state_.PpuBanks[1] = baseLow + 0x0400;
        state_.PpuBanks[2] = baseLow + 0x0800;
        state_.PpuBanks[3] = baseLow + 0x0c00;
        state_.PpuBanks[4] = baseHigh;
        state_.PpuBanks[5] = baseHigh + 0x0400;
        state_.PpuBanks[6] = baseHigh + 0x0800;
        state_.PpuBanks[7] = baseHigh + 0x0c00;
        break;
    }

    case 2:
    {
        uint8_t* base0;
        uint8_t* base1;
        uint8_t* base2;
        uint8_t* base3;
        if (useSecondary)
        {
            base0 = base2 = &chrData_[(state_.ChrBank3 << 11) & chrMask_];
            base1 = base3 = &chrData_[(state_.ChrBank7 << 11) & chrMask_];
        }
        else
        {
            base0 = &chrData_[(state_.ChrBank1 << 11) & chrMask_];
            base1 = &chrData_[(state_.ChrBank3 << 11) & chrMask_];
            base2 = &chrData_[(state_.ChrBank5 << 11) & chrMask_];
            base3 = &chrData_[(state_.ChrBank7 << 11) & chrMask_];
        }
        state_.PpuBanks[0] = base0;
        state_.PpuBanks[1] = base0 + 0x0400;
        state_.PpuBanks[2] = base1;
        state_.PpuBanks[3] = base1 + 0x0400;
        state_.PpuBanks[4] = base2;
        state_.PpuBanks[5] = base2 + 0x0400;
        state_.PpuBanks[6] = base3;
        state_.PpuBanks[7] = base3 + 0x0400;
        break;
    }

    case 3:
    {
        if (useSecondary)
        {
            state_.PpuBanks[0] = &chrData_[(state_.SecondaryChrBank0 << 10) & chrMask_];
            state_.PpuBanks[1] = &chrData_[(state_.SecondaryChrBank1 << 10) & chrMask_];
            state_.PpuBanks[2] = &chrData_[(state_.SecondaryChrBank2 << 10) & chrMask_];
            state_.PpuBanks[3] = &chrData_[(state_.SecondaryChrBank3 << 10) & chrMask_];
            state_.PpuBanks[4] = &chrData_[(state_.SecondaryChrBank0 << 10) & chrMask_];
            state_.PpuBanks[5] = &chrData_[(state_.SecondaryChrBank1 << 10) & chrMask_];
            state_.PpuBanks[6] = &chrData_[(state_.SecondaryChrBank2 << 10) & chrMask_];
            state_.PpuBanks[7] = &chrData_[(state_.SecondaryChrBank3 << 10) & chrMask_];
        }
        else
        {
            state_.PpuBanks[0] = &chrData_[(state_.ChrBank0 << 10) & chrMask_];
            state_.PpuBanks[1] = &chrData_[(state_.ChrBank1 << 10) & chrMask_];
            state_.PpuBanks[2] = &chrData_[(state_.ChrBank2 << 10) & chrMask_];
            state_.PpuBanks[3] = &chrData_[(state_.ChrBank3 << 10) & chrMask_];
            state_.PpuBanks[4] = &chrData_[(state_.ChrBank4 << 10) & chrMask_];
            state_.PpuBanks[5] = &chrData_[(state_.ChrBank5 << 10) & chrMask_];
            state_.PpuBanks[6] = &chrData_[(state_.ChrBank6 << 10) & chrMask_];
            state_.PpuBanks[7] = &chrData_[(state_.ChrBank7 << 10) & chrMask_];
        }
        break;
    }
    }
}

void Cart::UpdateNametableMapMMC5()
{
    UpdateNametableMMC5(0, state_.NametableMode0);
    UpdateNametableMMC5(1, state_.NametableMode1);
    UpdateNametableMMC5(2, state_.NametableMode2);
    UpdateNametableMMC5(3, state_.NametableMode3);
}

void Cart::UpdateNametableMMC5(uint32_t index, uint8_t mode)
{
    uint8_t* data = nullptr;
    switch (mode)
    {
    case 0:
        data = bus_->GetPpuRamBase();
        break;

    case 1:
        data = bus_->GetPpuRamBase() + 0x0400;
        break;

    case 2:
        if (state_.ExtendedRamMode < 2)
        {
            data = &state_.ExtendedRam[0];
        }
        else
        {
            state_.PpuBankFillBytes[index] = 0;
            state_.PPuBankAttributeBytes[index] = 0;
        }
        break;

    default:
        state_.PpuBankFillBytes[index] = state_.ChrFillValue;
        state_.PPuBankAttributeBytes[index] = state_.ChrFillAttributes;
        break;
    }

    state_.PpuBanks[8ULL + index] = state_.PpuBanks[12ULL + index] = data;
}

void Cart::WriteAxROM(uint16_t address, uint8_t value)
{
    if (busConflicts_)
    {
        auto bank = state_.CpuBanks[address >> 13];
        value &= bank[address & 0x1fff];
    }

    auto mirrorMode = (value & 0x10) ? MirrorMode::SingleScreenHigh : MirrorMode::SingleScreenLow;
    if (state_.MirrorMode != mirrorMode)
    {
        bus_->SyncPpu();
        SetMirrorMode(mirrorMode);
    }

    auto prgBank = &prgData_[((value & 0x07) << 15) & prgMask_];
    state_.CpuBanks[4] = prgBank;
    state_.CpuBanks[5] = prgBank + 0x2000;
    state_.CpuBanks[6] = prgBank + 0x4000;
    state_.CpuBanks[7] = prgBank + 0x6000;
}

void Cart::WriteMMC2(uint16_t address, uint8_t value)
{
    switch (address >> 12)
    {
    case 0xA:
    {
        auto prgBank = value & 0x0f;
        state_.CpuBanks[4] = &prgData_[(prgBank << 13) & prgMask_];
        break;
    }

    case 0xB:
        bus_->SyncPpu();
        state_.ChrBank0 = value & 0x1f;
        UpdateChrMapMMC2();
        break;

    case 0xC:
        bus_->SyncPpu();
        state_.SecondaryChrBank0 = value & 0x1f;
        UpdateChrMapMMC2();
        break;

    case 0xD:
        bus_->SyncPpu();
        state_.ChrBank1 = value & 0x1f;
        UpdateChrMapMMC2();
        break;

    case 0xE:
        bus_->SyncPpu();
        state_.SecondaryChrBank1 = value & 0x1f;
        UpdateChrMapMMC2();
        break;

    case 0xF:
        bus_->SyncPpu();
        SetMirrorMode((value & 0x01) ? MirrorMode::Horizontal : MirrorMode::Vertical);
        break;
    }
}

void Cart::PpuReadMMC2(uint16_t address)
{
    if (address == 0x0fd8)
    {
        if (state_.UseSecondaryChr0)
        {
            state_.UseSecondaryChr0 = false;
            UpdateChrMapMMC2();
        }
    }
    else if (address == 0x0fe8)
    {
        if (!state_.UseSecondaryChr0)
        {
            state_.UseSecondaryChr0 = true;
            UpdateChrMapMMC2();
        }
    }
    else if ((address & 0xfff8) == 0x1fd8)
    {
        if (state_.UseSecondaryChr1)
        {
            state_.UseSecondaryChr1 = false;
            UpdateChrMapMMC2();
        }
    }
    else if ((address & 0xfff8) == 0x1fe8)
    {
        if (!state_.UseSecondaryChr1)
        {
            state_.UseSecondaryChr1 = true;
            UpdateChrMapMMC2();
        }
    }
}

void Cart::UpdateChrMapMMC2()
{
    auto bank0 = state_.UseSecondaryChr0 ? state_.SecondaryChrBank0 : state_.ChrBank0;
    auto bank1 = state_.UseSecondaryChr1 ? state_.SecondaryChrBank1 : state_.ChrBank1;

    auto base0 = &chrData_[(bank0 << 12) & chrMask_];
    auto base1 = &chrData_[(bank1 << 12) & chrMask_];

    state_.PpuBanks[0] = base0;
    state_.PpuBanks[1] = base0 + 0x0400;
    state_.PpuBanks[2] = base0 + 0x0800;
    state_.PpuBanks[3] = base0 + 0x0c00;

    state_.PpuBanks[4] = base1;
    state_.PpuBanks[5] = base1 + 0x0400;
    state_.PpuBanks[6] = base1 + 0x0800;
    state_.PpuBanks[7] = base1 + 0x0c00;
}

void Cart::WriteColorDreams(uint16_t address, uint8_t value)
{
    if (busConflicts_)
    {
        auto bank = state_.CpuBanks[address >> 13];
        value &= bank[address & 0x1fff];
    }

    bus_->SyncPpu();

    auto cpuBase = &prgData_[((value & 0x03) << 15) & prgMask_];
    state_.CpuBanks[4] = cpuBase;
    state_.CpuBanks[5] = cpuBase + 0x2000;
    state_.CpuBanks[6] = cpuBase + 0x4000;
    state_.CpuBanks[7] = cpuBase + 0x6000;

    auto ppuBase = &chrData_[((value & 0xf0) << 9) & chrMask_];
    state_.PpuBanks[0] = ppuBase;
    state_.PpuBanks[1] = ppuBase + 0x0400;
    state_.PpuBanks[2] = ppuBase + 0x0800;
    state_.PpuBanks[3] = ppuBase + 0x0c00;
    state_.PpuBanks[4] = ppuBase + 0x1000;
    state_.PpuBanks[5] = ppuBase + 0x1400;
    state_.PpuBanks[6] = ppuBase + 0x1800;
    state_.PpuBanks[7] = ppuBase + 0x1c00;
}

void Cart::WriteCPROM(uint16_t address, uint8_t value)
{
    if (busConflicts_)
    {
        auto bank = state_.CpuBanks[address >> 13];
        value &= bank[address & 0x1fff];
    }

    bus_->SyncPpu();

    auto ppuBase = &chrData_[((value & 0x03) << 12) & chrMask_];
    state_.PpuBanks[4] = ppuBase;
    state_.PpuBanks[5] = ppuBase + 0x0400;
    state_.PpuBanks[6] = ppuBase + 0x0800;
    state_.PpuBanks[7] = ppuBase + 0x0c00;
}

void Cart::WriteNINA001(uint16_t address, uint8_t value)
{
    switch (address)
    {
    case 0x7ffd:
    {
        auto base = &prgData_[((value & 0x01) << 15) & prgMask_];
        state_.CpuBanks[4] = base;
        state_.CpuBanks[5] = base + 0x2000;
        state_.CpuBanks[6] = base + 0x4000;
        state_.CpuBanks[7] = base + 0x6000;
        break;
    }

    case 0x7ffe:
    {
        bus_->SyncPpu();
        auto base = &chrData_[((value & 0x0f) << 12) & chrMask_];
        state_.PpuBanks[0] = base;
        state_.PpuBanks[1] = base + 0x0400;
        state_.PpuBanks[2] = base + 0x0800;
        state_.PpuBanks[3] = base + 0x0c00;
        break;
    }

    case 0x7fff:
    {
        bus_->SyncPpu();
        auto base = &chrData_[((value & 0x0f) << 12) & chrMask_];
        state_.PpuBanks[4] = base;
        state_.PpuBanks[5] = base + 0x0400;
        state_.PpuBanks[6] = base + 0x0800;
        state_.PpuBanks[7] = base + 0x0c00;
        break;
    }
    }
}

void Cart::WriteBNROM(uint16_t address, uint8_t value)
{
    if (busConflicts_)
    {
        auto bank = state_.CpuBanks[address >> 13];
        value &= bank[address & 0x1fff];
    }

    auto base = &prgData_[((value & 0x03) << 15) & prgMask_];
    state_.CpuBanks[4] = base;
    state_.CpuBanks[5] = base + 0x2000;
    state_.CpuBanks[6] = base + 0x4000;
    state_.CpuBanks[7] = base + 0x6000;
}

void Cart::WriteCaltron6in1Low(uint16_t address)
{
    if ((address & 0xf800) != 0x6000)
        return;

    bus_->SyncPpu();

    SetMirrorMode(address & 0020 ? MirrorMode::Horizontal : MirrorMode::Vertical);
    state_.ChrBank0 = (address & 0x0018) << 12;
    state_.PrgBank0 = address & 0x0007;

    auto base = &prgData_[(state_.PrgBank0 << 15) & prgMask_];
    state_.CpuBanks[4] = base;
    state_.CpuBanks[5] = base + 0x2000;
    state_.CpuBanks[6] = base + 0x4000;
    state_.CpuBanks[7] = base + 0x6000;
}

void Cart::WriteCaltron6in1High(uint16_t address, uint8_t value)
{
    if (state_.PrgBank0 < 4)
        return;

    if (busConflicts_)
    {
        auto bank = state_.CpuBanks[address >> 13];
        value &= bank[address & 0x1fff];
    }

    bus_->SyncPpu();

    auto base = &chrData_[state_.ChrBank0 | ((value & 0x03) << 13)];
    state_.PpuBanks[0] = base;
    state_.PpuBanks[1] = base + 0x0400;
    state_.PpuBanks[2] = base + 0x0800;
    state_.PpuBanks[3] = base + 0x0c00;
    state_.PpuBanks[4] = base + 0x1000;
    state_.PpuBanks[5] = base + 0x1400;
    state_.PpuBanks[6] = base + 0x1800;
    state_.PpuBanks[7] = base + 0x1c00;
}

void Cart::SetChrBank1k(uint32_t bank, uint32_t value)
{
    auto bankAddress = (value << 10) & chrMask_;
    auto base = &chrData_[bankAddress];
    state_.PpuBanks[bank] = base;
}

void Cart::SetChrBank2k(uint32_t bank, uint32_t value)
{
    auto bankAddress = (value << 10) & chrMask_ & 0xfffff800;
    auto base = &chrData_[bankAddress];
    state_.PpuBanks[bank] = base;
    state_.PpuBanks[static_cast<size_t>(bank) + 1] = base + 0x0400;
}

void Cart::UpdatePpuRamMap()
{
    auto base = bus_->GetPpuRamBase();
    switch (state_.MirrorMode)
    {
    case MirrorMode::SingleScreenLow:
        state_.PpuBanks[8] = state_.PpuBanks[12] = base;
        state_.PpuBanks[9] = state_.PpuBanks[13] = base;
        state_.PpuBanks[10] = state_.PpuBanks[14] = base;
        state_.PpuBanks[11] = state_.PpuBanks[15] = base;
        break;

    case MirrorMode::SingleScreenHigh:
        state_.PpuBanks[8] = state_.PpuBanks[12] = base + 0x400;
        state_.PpuBanks[9] = state_.PpuBanks[13] = base + 0x400;
        state_.PpuBanks[10] = state_.PpuBanks[14] = base + 0x400;
        state_.PpuBanks[11] = state_.PpuBanks[15] = base + 0x400;
        break;

    case MirrorMode::Vertical:
        state_.PpuBanks[8] = state_.PpuBanks[12] = base;
        state_.PpuBanks[9] = state_.PpuBanks[13] = base + 0x400;
        state_.PpuBanks[10] = state_.PpuBanks[14] = base;
        state_.PpuBanks[11] = state_.PpuBanks[15] = base + 0x400;
        break;

    case MirrorMode::Horizontal:
        state_.PpuBanks[8] = state_.PpuBanks[12] = base;
        state_.PpuBanks[9] = state_.PpuBanks[13] = base;
        state_.PpuBanks[10] = state_.PpuBanks[14] = base + 0x400;
        state_.PpuBanks[11] = state_.PpuBanks[15] = base + 0x400;
        break;
    }
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

    if (desc.MirrorMode == MirrorMode::FourScreen)
    {
        // TODO:
        return nullptr;
    }

    cart->SetMirrorMode(desc.MirrorMode);

    MapperType mapper;
    switch (desc.Mapper)
    {
    case 0:
        if (desc.SubMapper != 0)
            return nullptr;
        mapper = MapperType::NROM;
        break;

    case 1:
        switch (desc.SubMapper)
        {
        case 0: // MMC1
        case 1: // SUROM
        case 2: // SOROM
        case 4: // SXROM
        case 5: // SEROM, SHROM, SH1ROM
                // TODO: disconnect A14 from MMC1?
            mapper = MapperType::MMC1;
            break;

        case 3:
            // mapper 155
            return nullptr;

        default:
            return nullptr;
        }
        break;

    case 2:
        mapper = MapperType::UxROM;

        switch (desc.SubMapper)
        {
        case 1:
            cart->EnableBusConflicts(false);
            break;

        case 0:
        case 2:
            cart->EnableBusConflicts(true);
            break;

        default:
            return nullptr;
        }
        break;

    case 3:
        mapper = MapperType::CNROM;

        switch (desc.SubMapper)
        {
        case 1:
            cart->EnableBusConflicts(false);
            break;

        case 0:
        case 2:
            cart->EnableBusConflicts(true);
            break;

        default:
            return nullptr;
        }
        break;

    case 4:
        switch (desc.SubMapper)
        {
        case 0:
            mapper = MapperType::MMC3;
            break;

        case 1: // MMC6
            mapper = MapperType::MMC6;
            break;

        case 3: // MMC-ACC
            mapper = MapperType::MCACC;
            break;

        case 2: // deprecated
        case 4: // MMC3A
        default:
            return nullptr; 
        }
        break;

    case 5:
        mapper = MapperType::MMC5;
        break;

    case 7:
        mapper = MapperType::AxROM;

        switch (desc.SubMapper)
        {
        case 0:
        case 1:
            cart->EnableBusConflicts(false);
            break;

        case 2:
            cart->EnableBusConflicts(true);
            break;

        default:
            return nullptr;
        }
        break;

    case 9:
        mapper = MapperType::MMC2;
        break;

    case 11:
        mapper = MapperType::ColorDreams;
        cart->EnableBusConflicts(true);
        break;

    case 13:
        mapper = MapperType::CPROM;
        cart->EnableBusConflicts(true);
        break;

    case 34:
        switch (desc.SubMapper)
        {
        case 0:
            if (chrData.size() > 0x2000)
            {
                mapper = MapperType::NINA001;
                break;
            }
            else
            {
                mapper = MapperType::BNROM;
                cart->EnableBusConflicts(true);
                break;
            }

        case 1:
            mapper = MapperType::NINA001;
            break;

        case 2:
            mapper = MapperType::BNROM;
            cart->EnableBusConflicts(true);
            break;

        default:
            return nullptr;
        }

    case 41:
        mapper = MapperType::Caltron6in1;
        cart->EnableBusConflicts(true);
        break;

    case 71:
        switch (desc.SubMapper)
        {
        case 0:
            mapper = MapperType::BF9093;
            break;
        }
        break;

    default:
        return nullptr;
    }

    cart->SetMapper(mapper);

    if (desc.PrgRamSize != 0 && desc.PrgBatteryRamSize != 0 && desc.PrgRamSize != desc.PrgBatteryRamSize)
        return nullptr;

    if (desc.PrgBatteryRamSize != 0)
    {
        if (batteryRam)
            cart->AddPrgRam(batteryRam, desc.PrgBatteryRamSize);
        else
            cart->AddPrgRam(desc.PrgBatteryRamSize);
    }

    if (desc.PrgRamSize != 0)
        cart->AddPrgRam(desc.PrgRamSize);

    if (desc.ChrRamSize != 0 && chrData.size() != 0)
        return nullptr;

    if (desc.ChrRamSize != 0)
        cart->SetChrRam(desc.ChrRamSize);

    return std::move(cart);
}