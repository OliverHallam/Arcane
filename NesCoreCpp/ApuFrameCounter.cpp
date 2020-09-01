#include "ApuFrameCounter.h"

#include "Apu.h"

#include <cassert>

ApuFrameCounter::ApuFrameCounter(Apu& apu)
    : apu_{ apu },
    phase_{ 0 }
{
}

void ApuFrameCounter::EnableInterrupt(bool enable)
{
    enableInterrupt_ = enable;
    if (!enable)
    {
        apu_.SetFrameCounterInterrupt(false);
    }
}

uint32_t ApuFrameCounter::SetMode(uint8_t mode)
{
    mode_ = mode;

    if (mode == 1)
    {
        // we will reset in 3/4 cycles time so lets do this by jumping to the end 
        phase_ = 6;
        return 3;
    }
    else
    {
        // if the mode flag is not set, we don't generate the quarter and half frame signals.
        phase_ = 0;
        return 7460;
    }
}

uint32_t ApuFrameCounter::Activate()
{
    switch (phase_)
    {
    case 0:
        apu_.QuarterFrame();
        phase_ = 1;
        return 7456;

    case 1:
        apu_.QuarterFrame();
        apu_.HalfFrame();
        phase_ = 2;
        return 7458;

    case 2:
        apu_.QuarterFrame();
        if (mode_ == 0)
        {
            // phases 3,4,5 are the final step for the 4 step sequence.
            phase_ = 3;
            return 7457;
        }
        else
        {
            // phase 6 is the final step for the 5 step sequence (step 4 has no effects)
            phase_ = 6;
            return 14910;
        }

    case 3:
        if (enableInterrupt_)
            apu_.SetFrameCounterInterrupt(true);

        phase_ = 4;
        return 1;

    case 4:
        if (enableInterrupt_)
            apu_.SetFrameCounterInterrupt(true);

        apu_.QuarterFrame();
        apu_.HalfFrame();
        phase_ = 5;
        return 1;

    case 5:
        if (enableInterrupt_)
            apu_.SetFrameCounterInterrupt(true);

        phase_ = 0;
        return 7456;

    case 6:
        apu_.QuarterFrame();
        apu_.HalfFrame();
        phase_ = 0;
        return 7457;

    default:
        assert(false);
        return 0xffffffff;
    }
}
