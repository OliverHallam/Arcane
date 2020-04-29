#include "ApuDmc.h"

#include "Bus.h"

ApuDmc::ApuDmc(Apu& apu)
    : enabled_{},
    irqEnabled_{},
    loop_{},
    rate_{ 428 },
    level_{},
    sampleAddress_{},
    sampleLength_{},
    timer_{ 428 },
    outBuffer_{},
    outBufferHasData_{},
    sampleShift_{ 0 },
    sampleBuffer_{},
    sampleBufferHasData_{},
    currentAddress_{},
    sampleBytesRemaining_{},
    apu_{ apu }
{
}

void ApuDmc::Enable(bool enabled)
{
    enabled_ = enabled;
}

bool ApuDmc::IsEnabled() const
{
    return enabled_;
}

void ApuDmc::Write(uint16_t address, uint8_t value)
{
    switch (address & 0x0003)
    {
    case 0:
        irqEnabled_ = value & 0x80;
        loop_ = value & 0x40;
        rate_ = GetLinearRate(value & 0x0f);
        if (!irqEnabled_)
            apu_.SetDmcInterrupt(false);
        return;

    case 1:
        // TODO: this can be missed if a clock is happening at the same time
        level_ = value & 0x7f;
        return;

    case 2:
        currentAddress_ = sampleAddress_ = 0xc000 | (value << 6);
        return;

    case 3:
        sampleBytesRemaining_ = sampleBytesRemaining_ = (value << 4) + 1;
        break;
    }
}

void ApuDmc::Run(uint32_t cycles)
{
    timer_ -= cycles;

    while (timer_ <= 0)
    {
        Clock();
        timer_ += rate_;
    }
}

void ApuDmc::SetBuffer(uint8_t value)
{
    sampleBuffer_ = value;
    sampleBufferHasData_ = true;
}

uint8_t ApuDmc::Sample() const
{
    return level_;
}

uint32_t ApuDmc::CyclesUntilNextDma()
{
    // TODO: this could be tighter - if we've raised the IRQ there's nothing more we need to sync with the bus.
    if (!sampleBytesRemaining_ && !irqEnabled_)
        return 0xffffffff;

    return timer_ + (7 - sampleShift_) * rate_;
}

void ApuDmc::Clock()
{
    if (outBufferHasData_)
    {
        auto nextBit = (outBuffer_ >> sampleShift_) & 1;

        if (nextBit)
        {
            if (level_ <= 125)
                level_ += 2;
        }
        else
        {
            if (level_ >= 2)
                level_ -= 2;
        }
    }

    if (sampleShift_ == 7)
    {
        sampleShift_ = 0;
        outBuffer_ = sampleBuffer_;
        outBufferHasData_ = sampleBufferHasData_;
        sampleBufferHasData_ = false;

        if (sampleBytesRemaining_ == 0)
        {
            return;
        }

        apu_.RequestDmcByte(currentAddress_);
        currentAddress_++;
        currentAddress_ |= 0x8000;
        sampleBytesRemaining_--;

        if (sampleBytesRemaining_ == 0)
        {
            if (!loop_)
            {
                outBuffer_ = 0;
                if (irqEnabled_)
                    apu_.SetDmcInterrupt(true);
                return;
            }

            sampleBytesRemaining_ = sampleLength_;
            currentAddress_ = sampleAddress_;
        }
    }
    else
        sampleShift_++;
}

uint32_t ApuDmc::GetLinearRate(uint8_t rate)
{
    switch (rate)
    {
    case 0: return 428;
    case 1: return 380;
    case 2: return 340;
    case 3: return 320;
    case 4: return 286;
    case 5: return 254;
    case 6: return 226;
    case 7: return 214;
    case 8: return 190;
    case 9: return 160;
    case 10: return 142;
    case 11: return 128;
    case 12: return 106;
    case 13: return 84;
    case 14: return 72;
    case 15: return 54;
    }

    return 0;
}
