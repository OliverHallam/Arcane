#include "ApuSweep.h"

void ApuSweep::SetPeriodLow(uint8_t value)
{
    period_ &= 0x0700;
    period_ |= value;
    period2_ = period_ * 2 + 2;
    UpdateTargetPeriod();
}

void ApuSweep::SetSweep(uint8_t value)
{
    enabled_ = (value & 0x80) != 0;
    divide_ = (value & 0x70) >> 4;
    negate_ = (value & 0x08) != 0;
    shift_ = value & 0x07;
    reload_ = true;
    UpdateTargetPeriod();
}

void ApuSweep::Tick()
{
    if (!divideCounter_)
    {
        if (enabled_ && IsOutputEnabled())
        {
            period_ = targetPeriod_;
            period2_ = period_ * 2 + 2;
            UpdateTargetPeriod();
        }

        divideCounter_ = divide_;
        reload_ = false;
    }
    else if (reload_)
    {
        divideCounter_ = divide_;
        reload_ = false;
    }
    else
    {
        divideCounter_--;
    }
}

uint16_t ApuSweep::Period() const
{
    return period2_;
}

bool ApuSweep::IsOutputEnabled() const
{
    // negate can cause an underflow
    return period_ >= 8 && targetPeriod_ <= 0x800;
}

void ApuSweep::UpdateTargetPeriod()
{
    auto delta = period_ >> shift_;

    if (negate_)
    {
        delta = -delta;
        if (delta > 0)
            delta -= negatedDeltaOffset_;
    }

    targetPeriod_ = period_ + delta;
}

ApuSweep::ApuSweep(bool pulse1)
{
    negatedDeltaOffset_ = pulse1 ? 1 : 0;
}

void ApuSweep::SetPeriodHigh(uint8_t value)
{
    period_ &= 0x00ff;
    period_ |= value << 8;
    period2_ = period_ * 2 + 2;
    UpdateTargetPeriod();
}
