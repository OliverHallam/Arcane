#include "ApuNoise.h"

ApuNoise::ApuNoise()
    : modeShift_{1},
    period_{},
    period2_{2},
    timer_{},
    shifter_{1}
{
}

void ApuNoise::Disable()
{
    lengthCounter_.Disable();
}

bool ApuNoise::IsEnabled() const
{
    return lengthCounter_.IsEnabled();
}

void ApuNoise::Write(uint16_t address, uint8_t value)
{
    switch (address)
    {
    case 0:
        lengthCounter_.SetHalt(value & 0x20);
        envelope_.SetConstantVolume(value & 0x10);
        envelope_.SetValue(value & 0x0f);
        break;

    case 2:
        modeShift_ = value >> 7 ? 6 : 1;
        period_ = LookupPeriod(value & 0x0f);
        period2_ = period_ * 2 + 2;
        break;

    case 3:
        lengthCounter_.SetLength(value >> 3);
        envelope_.Start();
        break;
    }
}

void ApuNoise::Run(uint32_t cycles)
{
    timer_ -= cycles;
    while (timer_ <= 0)
    {
        StepSequencer();
        timer_ += period2_;
    }
}

void ApuNoise::TickQuarterFrame()
{
    envelope_.Tick();
}

void ApuNoise::TickHalfFrame()
{
    lengthCounter_.Tick();
}

int8_t ApuNoise::Sample() const
{
    if (lengthCounter_.IsEnabled())
        return GetSequenceOutput() ? envelope_.Sample() : -envelope_.Sample();

    return 0;
}

void ApuNoise::StepSequencer()
{
    auto feedback = (shifter_ ^ (shifter_ >> modeShift_)) & 0x0001;
    shifter_ = (shifter_ >> 1) | (feedback << 14);
}

uint_fast16_t ApuNoise::LookupPeriod(uint8_t period)
{
    switch (period)
    {
    case 0x00: return 4;
    case 0x01: return 8;
    case 0x02: return 16;
    case 0x03: return 32;
    case 0x04: return 64;
    case 0x05: return 96;
    case 0x06: return 128;
    case 0x07: return 160;
    case 0x08: return 202;
    case 0x09: return 254;
    case 0x0a: return 380;
    case 0x0b: return 508;
    case 0x0c: return 762;
    case 0x0d: return 1016;
    case 0x0e: return 2034;
    case 0x0f: return 4068;

    default: return 0;
    }
}

bool ApuNoise::GetSequenceOutput() const
{
    return !(shifter_ & 0x0001);
}
