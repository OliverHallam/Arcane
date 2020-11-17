#pragma once

#include "ApuLengthCounterState.h"
#include "ApuTriangleCoreState.h"

struct ApuTriangleState
{
    ApuLengthCounterState LengthCounter;

    ApuTriangleCoreState Core;
};