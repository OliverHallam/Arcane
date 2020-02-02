#include "ApuFrameCounter.h"

#include "Apu.h"

ApuFrameCounter::ApuFrameCounter(Apu& apu)
    : apu_{ apu }
{
}

void ApuFrameCounter::SetMode(uint8_t mode)
{
    mode_ = mode;

    // we will reset in 3/4 cycles time so lets do this by jumping to the end 
    if (cycleCount_ & 1)
        cycleCount_ = 37277;
    else
        cycleCount_ = 37278;
}

void ApuFrameCounter::Tick()
{
    // MODE 0:
    switch (cycleCount_++)
    {
    case 7457:
        apu_.QuarterFrame();
        break;

    case 14913:
        apu_.QuarterFrame();
        apu_.HalfFrame();
        break;

    case 22371:
        apu_.QuarterFrame();
        break;

    case 29829:
        if (mode_ == 0)
        {
            // TODO: interrupt
            apu_.QuarterFrame();
            apu_.HalfFrame();
            cycleCount_ = 0;
        }
        break;

    case 37281:
        if (mode_ == 1)
        {
            apu_.QuarterFrame();
            apu_.HalfFrame();
            cycleCount_ = 0;
        }
        break;
    }
}
