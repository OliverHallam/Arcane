#include "ApuPulse.h"

ApuPulse::ApuPulse(bool pulse1)
    : sweep_{pulse1}
{
}

void ApuPulse::Run(uint32_t cycles)
{
    timer_ -= cycles;
    while (timer_ <= 0)
    {
        timer_ += sweep_.Period();
        sequence_--;
    }
}

void ApuPulse::TickQuarterFrame()
{
    envelope_.Tick();
}

void ApuPulse::TickHalfFrame()
{
    sweep_.Tick();
    lengthCounter_.Tick();
}

void ApuPulse::Enabled(bool enabled)
{
    lengthCounter_.SetEnabled(enabled);
}

bool ApuPulse::IsEnabled() const
{
    return lengthCounter_.IsEnabled();
}

void ApuPulse::Write(uint8_t address, uint8_t value)
{
    switch (address)
    {
    case 0:
        dutyLookup_ = GetDutyLookup(value >> 6);
        lengthCounter_.SetHalt(value & 0x20);
        envelope_.SetLoop(value & 0x20);
        envelope_.SetConstantVolume(value & 0x10);
        envelope_.SetValue(value & 0x0f);
        break;

    case 1:
        sweep_.SetSweep(value);
        break;

    case 2:
        sweep_.SetPeriodLow(value);
        break;

    case 3:
        lengthCounter_.SetLength((value & 0xf8) >> 3);
        sweep_.SetPeriodHigh(value & 0x07);
        sequence_ = 0;
        envelope_.Start();
        break;
    }
}

int8_t ApuPulse::Sample() const
{
    if (lengthCounter_.IsEnabled() && sweep_.IsOutputEnabled())
        return GetSequenceOutput() ? envelope_.Sample() : -envelope_.Sample();

    return 0;
}

uint8_t ApuPulse::GetDutyLookup(uint8_t duty)
{
    switch (duty)
    {
    case 0: return 0x01;
    case 1: return 0x03;
    case 2: return 0x0f;
    case 3: return 0xfc;

    default: return 0;
    }
}

bool ApuPulse::GetSequenceOutput() const
{
    return (dutyLookup_ << (sequence_ & 0x07)) & 0x80;
}
