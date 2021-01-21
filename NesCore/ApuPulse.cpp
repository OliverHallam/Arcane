#include "ApuPulse.h"

ApuPulse::ApuPulse(bool pulse1)
    : sweep_{pulse1}
{
}

void ApuPulse::Run(uint32_t cycles)
{
    state_.timer_ -= cycles;
    while (state_.timer_ <= 0)
    {
        state_.timer_ += sweep_.Period();
        state_.sequence_--;
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

void ApuPulse::Write(uint16_t address, uint8_t value)
{
    switch (address & 0x0003)
    {
    case 0:
        state_.dutyLookup_ = GetDutyLookup(value >> 6);
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
        state_.sequence_ = 0;
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

void ApuPulse::CaptureState(ApuPulseState* state) const
{
    envelope_.CaptureState(&state->Envelope);
    lengthCounter_.CaptureState(&state->LengthCounter);
    sweep_.CaptureState(&state->Sweep);

    state->Core = state_;
}

void ApuPulse::RestoreState(const ApuPulseState& state)
{
    envelope_.RestoreState(state.Envelope);
    lengthCounter_.RestoreState(state.LengthCounter);
    sweep_.RestoreState(state.Sweep);

    state_ = state.Core;
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
    return (state_.dutyLookup_ << (state_.sequence_ & 0x07)) & 0x80;
}
