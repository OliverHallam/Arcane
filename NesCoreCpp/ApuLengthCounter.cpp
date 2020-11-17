#include "ApuLengthCounter.h"

void ApuLengthCounter::Tick()
{
    if (state_.Length && !state_.Halt)
        --state_.Length;
}

void ApuLengthCounter::SetHalt(bool halt)
{
    state_.Halt = halt;
}

void ApuLengthCounter::SetLength(uint8_t length)
{
    if (state_.Enabled)
        state_.Length = GetLinearLength(length);
}

void ApuLengthCounter::SetEnabled(bool enabled)
{
    state_.Enabled = enabled;
    if (!enabled)
        state_.Length = 0;
}

bool ApuLengthCounter::IsEnabled() const
{
    return state_.Length > 0;
}

void ApuLengthCounter::CaptureState(ApuLengthCounterState* state) const
{
    *state = state_;
}

void ApuLengthCounter::RestoreState(const ApuLengthCounterState& state)
{
    state_ = state;
}

uint8_t ApuLengthCounter::GetLinearLength(uint8_t length)
{
    switch (length)
    {
    case 0x00: return 10;
    case 0x02: return 20;
    case 0x04: return 40;
    case 0x06: return 80;
    case 0x08: return 160;
    case 0x0a: return 60;
    case 0x0c: return 14;
    case 0x0e: return 26;

    case 0x10: return 12;
    case 0x12: return 24;
    case 0x14: return 48;
    case 0x16: return 96;
    case 0x18: return 192;
    case 0x1a: return 72;
    case 0x1c: return 16;
    case 0x1e: return 32;

    case 0x01: return 254;
    case 0x03: return 2;
    case 0x05: return 4;
    case 0x07: return 6;
    case 0x09: return 8;
    case 0x0b: return 10;
    case 0x0d: return 12;
    case 0x0f: return 14;
    case 0x11: return 16;
    case 0x13: return 18;
    case 0x15: return 20;
    case 0x17: return 22;
    case 0x19: return 24;
    case 0x1b: return 26;
    case 0x1d: return 28;
    case 0x1f: return 30;

    default: return 0;
    }
}

