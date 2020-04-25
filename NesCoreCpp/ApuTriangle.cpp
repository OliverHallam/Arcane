#include "ApuTriangle.h"

void ApuTriangle::Disable()
{
    lengthCounter_.Disable();
}

bool ApuTriangle::IsEnabled() const
{
    return lengthCounter_.IsEnabled();
}

void ApuTriangle::Write(uint16_t address, uint8_t value)
{
    switch (address)
    {
    case 0:
        control_ = value & 0x80;
        lengthCounter_.SetHalt(control_);
        linearCounterReloadValue_ = value & 0x7f;
        break;

    case 2:
        period_ &= 0x0700;
        period_ |= value;
        period2_ = period_ + 1;
        break;

    case 3:
        lengthCounter_.SetLength(value >> 3);
        period_ &= 0x00ff;
        period_ |= (value & 0x07) << 8;
        period2_ = period_ + 1;
        linearCounterReload_ = true;
        break;
    }
}

void ApuTriangle::Run(uint32_t cycles)
{
    timer_ -= cycles;

    if (linearCounter_ && lengthCounter_.IsEnabled())
    {
        while (timer_ <= 0)
        {
            waveformCycle_++;
            timer_ += period2_;
        }

        waveformCycle_ &= 0x1f;
    }
    else
    {
        while (timer_ <= 0)
        {
            timer_ += period2_;
        }
    }
}

void ApuTriangle::TickQuarterFrame()
{
    if (linearCounterReload_)
    {
        linearCounter_ = linearCounterReloadValue_;
    }
    else
    {
        if (linearCounter_)
        {
            linearCounter_--;
        }
    }

    if (!control_)
    {
        linearCounterReload_ = false;
    }
}

void ApuTriangle::TickHalfFrame()
{
    lengthCounter_.Tick();
}

int8_t ApuTriangle::Sample() const
{
    // the original hardware outputs from 0 - 15, I've scaled this to -15 - 15
    if (linearCounter_ && lengthCounter_.IsEnabled())
    {
        if (waveformCycle_ < 16)
            return 15 - waveformCycle_ * 2;
        else
            return waveformCycle_ * 2 - 47;
    }

    return 0;
}
