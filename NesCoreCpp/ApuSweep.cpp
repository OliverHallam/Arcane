#include "ApuSweep.h"

void ApuSweep::SetPeriodLow(uint8_t value)
{
    state_.Period &= 0x0700;
    state_.Period |= value;
    state_.Period2 = state_.Period * 2 + 2;
    UpdateTargetPeriod();
}

void ApuSweep::SetSweep(uint8_t value)
{
    state_.Enabled = (value & 0x80) != 0;
    state_.Divide = (value & 0x70) >> 4;
    state_.Negate = (value & 0x08) != 0;
    state_.Shift = value & 0x07;
    state_.Reload = true;
    UpdateTargetPeriod();
}

void ApuSweep::Tick()
{
    if (!state_.DivideCounter)
    {
        if (state_.Enabled && IsOutputEnabled() && state_.Shift)
        {
            state_.Period = state_.TargetPeriod;
            state_.Period2 = state_.Period * 2 + 2;
            UpdateTargetPeriod();
        }

        state_.DivideCounter = state_.Divide;
        state_.Reload = false;
    }
    else if (state_.Reload)
    {
        state_.DivideCounter = state_.Divide;
        state_.Reload = false;
    }
    else
    {
        state_.DivideCounter--;
    }
}

uint16_t ApuSweep::Period() const
{
    return state_.Period2;
}

bool ApuSweep::IsOutputEnabled() const
{
    // negate can cause an underflow
    return state_.Period >= 8 && state_.TargetPeriod < 0x800;
}

void ApuSweep::CaptureState(ApuSweepState* state) const
{
    *state = state_;
}

void ApuSweep::RestoreState(const ApuSweepState& state)
{
    state_ = state;
}

void ApuSweep::UpdateTargetPeriod()
{
    auto delta = state_.Period >> state_.Shift;

    if (state_.Negate)
    {
        delta = -delta;
        if (delta > 0)
            delta -= negatedDeltaOffset_;
    }

    state_.TargetPeriod = state_.Period + delta;
}

ApuSweep::ApuSweep(bool pulse1)
{
    negatedDeltaOffset_ = pulse1 ? 1 : 0;
}

void ApuSweep::SetPeriodHigh(uint8_t value)
{
    state_.Period &= 0x00ff;
    state_.Period |= value << 8;
    state_.Period2 = state_.Period * 2 + 2;
    UpdateTargetPeriod();
}
