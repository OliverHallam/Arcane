#pragma once

#include "ApuCoreState.h"
#include "ApuDmcState.h"
#include "ApuFrameCounterState.h"
#include "ApuNoiseState.h"
#include "ApuPulseState.h"
#include "ApuTriangleState.h"

struct ApuState
{
    ApuFrameCounterState FrameCounter;
    ApuPulseState Pulse1;
    ApuPulseState Pulse2;
    ApuTriangleState Triangle;
    ApuNoiseState Noise;
    ApuDmcState Dmc;

    ApuCoreState Core;

    // TODO: we may be able to strip this.
    std::unique_ptr<int16_t[]> BackBuffer;
};