#include "ApuFrameCounter.h"

#include "Apu.h"

ApuFrameCounter::ApuFrameCounter(Apu& apu)
    : apu_{ apu },
    counter_{ 7457 },
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

void ApuFrameCounter::SetMode(uint8_t mode)
{
    mode_ = mode;

    if (mode == 1)
    {
        // we will reset in 3/4 cycles time so lets do this by jumping to the end 
        phase_ = 4;
        if (counter_ & 1)
            counter_ = 3;
        else
            counter_ = 4;
    }
    else
    {
        // if the mode flag is not set, we don't generate the quarter and half frame signals.
        phase_ = 0;
        if (counter_ & 1)
            counter_ = 7460;
        else
            counter_ = 7461;
    }
}

void ApuFrameCounter::Tick()
{
    if (!--counter_)
        Activate();
}

void ApuFrameCounter::Activate()
{
    switch (phase_)
    {
    case 0:
        apu_.QuarterFrame();
        phase_ = 1;
        counter_ = 7456;
        break;

    case 1:
        apu_.QuarterFrame();
        apu_.HalfFrame();
        phase_ = 2;
        counter_ = 7458;
        break;

    case 2:
        apu_.QuarterFrame();
        if (mode_ == 0)
        {
            // phases 3,4,5 are the final step for the 4 step sequence.
            phase_ = 3;
            counter_ = 7457;
        }
        else
        {
            // phase 6 is the final step for the 5 step sequence (step 4 has no effects)
            phase_ = 6;
            counter_ = 14910;
        }
        break;

    case 3:
        if (enableInterrupt_)
            apu_.SetFrameCounterInterrupt(true);

        phase_ = 4;
        counter_ = 1;
        break;

    case 4:
        if (enableInterrupt_)
            apu_.SetFrameCounterInterrupt(true);

        apu_.QuarterFrame();
        apu_.HalfFrame();
        phase_ = 5;
        counter_ = 1;
        break;

    case 5:
        if (enableInterrupt_)
            apu_.SetFrameCounterInterrupt(true);

        phase_ = 0;
        counter_ = 7456;
        break;

    case 6:
        apu_.QuarterFrame();
        apu_.HalfFrame();
        phase_ = 0;
        counter_ = 7457;
        break;
    }
}
