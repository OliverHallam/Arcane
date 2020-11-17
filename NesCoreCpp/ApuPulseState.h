#pragma once

#include "ApuEnvelopeState.h"
#include "ApuLengthCounterState.h"
#include "ApuPulseCoreState.h"
#include "ApuSweepState.h"

struct ApuPulseState
{
    ApuEnvelopeState Envelope;
    ApuLengthCounterState LengthCounter;
    ApuSweepState Sweep;

    ApuPulseCoreState Core;
};