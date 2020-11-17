#pragma once

#include "PpuCoreState.h"
#include "PpuBackgroundState.h"
#include "PpuSpritesState.h"

struct PpuState
{
    PpuCoreState Core;
    PpuBackgroundState Background;
    PpuSpritesState Sprites;
};