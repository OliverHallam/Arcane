#include "ApuFrameCounter.h"

#include "Apu.h"

ApuFrameCounter::ApuFrameCounter(Apu& apu)
    : apu_{ apu },
    counter_{ 7457 },
    phase_{ 0 }
{
}

void ApuFrameCounter::SetMode(uint8_t mode)
{
    mode_ = mode;

    // we will reset in 3/4 cycles time so lets do this by jumping to the end 
    phase_ = 4;
    if (counter_ & 1)
        counter_ = 4;
    else
        counter_ = 3;
}

void ApuFrameCounter::Tick()
{
    if (!counter_--)
    {
        Activate();
    }
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
        phase_ = 3;
        counter_ = 7458;
        break;

    case 3:
        if (mode_ == 0)
        {
            apu_.QuarterFrame();
            apu_.HalfFrame();
            phase_ = 0;
            counter_ = 7458;
            break;
        }
        else
        {
            phase_ = 4;
            counter_ = 7452;
            break;
        }

    case 4:
        apu_.QuarterFrame();
        apu_.HalfFrame();
        phase_ = 0;
        counter_ = 7458;
        break;
    }
}
