#include "ApuFrameCounter.h"

#include "Apu.h"

ApuFrameCounter::ApuFrameCounter(Apu& apu)
    : apu_{ apu },
    cycleCount_{0}
{
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
        apu_.QuarterFrame();
        apu_.HalfFrame();
        cycleCount_ = 0;
        break;
    }
}
