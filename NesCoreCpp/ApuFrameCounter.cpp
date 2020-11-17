#include "ApuFrameCounter.h"

#include "Apu.h"

#include <cassert>

ApuFrameCounter::ApuFrameCounter(Apu& apu)
    : apu_{ apu }
{
}

void ApuFrameCounter::EnableInterrupt(bool enable)
{
    state_.EnableInterrupt = enable;
    if (!enable)
    {
        apu_.SetFrameCounterInterrupt(false);
    }
}

uint32_t ApuFrameCounter::SetMode(uint8_t mode)
{
    state_.Mode = mode;

    if (mode == 1)
    {
        // we will reset in 3/4 cycles time so lets do this by jumping to the end 
        state_.Phase = 6;
        return 3;
    }
    else
    {
        // if the mode flag is not set, we don't generate the quarter and half frame signals.
        state_.Phase = 0;
        return 7460;
    }
}

uint32_t ApuFrameCounter::Activate()
{
    switch (state_.Phase)
    {
    case 0:
        apu_.QuarterFrame();
        state_.Phase = 1;
        return 7456;

    case 1:
        apu_.QuarterFrame();
        apu_.HalfFrame();
        state_.Phase = 2;
        return 7458;

    case 2:
        apu_.QuarterFrame();
        if (state_.Mode == 0)
        {
            // phases 3,4,5 are the final step for the 4 step sequence.
            state_.Phase = 3;
            return 7457;
        }
        else
        {
            // phase 6 is the final step for the 5 step sequence (step 4 has no effects)
            state_.Phase = 6;
            return 14910;
        }

    case 3:
        if (state_.EnableInterrupt)
            apu_.SetFrameCounterInterrupt(true);

        state_.Phase = 4;
        return 1;

    case 4:
        if (state_.EnableInterrupt)
            apu_.SetFrameCounterInterrupt(true);

        apu_.QuarterFrame();
        apu_.HalfFrame();
        state_.Phase = 5;
        return 1;

    case 5:
        if (state_.EnableInterrupt)
            apu_.SetFrameCounterInterrupt(true);

        state_.Phase = 0;
        return 7456;

    case 6:
        apu_.QuarterFrame();
        apu_.HalfFrame();
        state_.Phase = 0;
        return 7457;

    default:
        assert(false);
        return 0xffffffff;
    }
}

void ApuFrameCounter::CaptureState(ApuFrameCounterState* state) const
{
    *state = state_;
}

void ApuFrameCounter::RestoreState(const ApuFrameCounterState& state)
{
    state_ = state;
}
