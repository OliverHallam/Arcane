#include "ApuTriangle.h"

void ApuTriangle::Enable(bool enabled)
{
    lengthCounter_.SetEnabled(enabled);
}

bool ApuTriangle::IsEnabled() const
{
    return lengthCounter_.IsEnabled();
}

void ApuTriangle::Write(uint16_t address, uint8_t value)
{
    switch (address & 0x0003)
    {
    case 0:
        state_.Control = value & 0x80;
        lengthCounter_.SetHalt(state_.Control);
        state_.LinearCounterReloadValue = value & 0x7f;
        break;

    case 2:
        state_.Period &= 0x0700;
        state_.Period |= value;
        state_.Period2 = state_.Period + 1;
        break;

    case 3:
        lengthCounter_.SetLength(value >> 3);
        state_.Period &= 0x00ff;
        state_.Period |= (value & 0x07) << 8;
        state_.Period2 = state_.Period + 1;
        state_.LinearCounterReload = true;
        break;
    }
}

void ApuTriangle::Run(uint32_t cycles)
{
    state_.Timer -= cycles;

    if (state_.LinearCounter && lengthCounter_.IsEnabled())
    {
        while (state_.Timer <= 0)
        {
            state_.WaveformCycle++;
            state_.Timer += state_.Period2;
        }

        state_.WaveformCycle &= 0x1f;
    }
    else
    {
        while (state_.Timer <= 0)
        {
            state_.Timer += state_.Period2;
        }
    }
}

void ApuTriangle::TickQuarterFrame()
{
    if (state_.LinearCounterReload)
    {
        state_.LinearCounter = state_.LinearCounterReloadValue;
    }
    else
    {
        if (state_.LinearCounter)
        {
            state_.LinearCounter--;
        }
    }

    if (!state_.Control)
    {
        state_.LinearCounterReload = false;
    }
}

void ApuTriangle::TickHalfFrame()
{
    lengthCounter_.Tick();
}

int8_t ApuTriangle::Sample() const
{
    // the original hardware outputs from 0 - 15, I've scaled this to -15 - 15

    // period check simulates the low-pass filter
    if (state_.Period2 <= 2)
        return 0;

    if (state_.WaveformCycle < 16)
        return 15 - state_.WaveformCycle * 2;
    else
        return state_.WaveformCycle * 2 - 47;
}

void ApuTriangle::CaptureState(ApuTriangleState* state) const
{
    lengthCounter_.CaptureState(&state->LengthCounter);

    state->Core = state_;
}

void ApuTriangle::RestoreState(const ApuTriangleState& state)
{
    lengthCounter_.RestoreState(state.LengthCounter);

    state_ = state.Core;
}
