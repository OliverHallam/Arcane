#pragma once

#include "ApuState.h"
#include "BusState.h"
#include "CartState.h"
#include "ControllerState.h"
#include "CpuState.h"
#include "PpuState.h"

struct SystemState
{
    BusState BusState;
    CpuState CpuState;
    PpuState PpuState;
    ApuState ApuState;
    ControllerState ControllerState;
    CartState CartState;
};