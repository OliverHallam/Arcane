#pragma once

#include "ApuEnvelopeState.h"
#include "ApuLengthCounterState.h"
#include "ApuNoiseCoreState.h"

struct ApuNoiseState
{
    ApuEnvelopeState Envelope;
    ApuLengthCounterState LengthCounter;

    ApuNoiseCoreState Core;
};