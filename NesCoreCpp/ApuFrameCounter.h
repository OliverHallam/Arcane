#pragma once

#include <cstdint>

#include "ApuFrameCounterState.h"

class Apu;

class ApuFrameCounter
{
public:
    ApuFrameCounter(Apu& apu);

    void EnableInterrupt(bool enable);
    uint32_t SetMode(uint8_t mode);

    uint32_t Activate();

    void CaptureState(ApuFrameCounterState* state) const;
    void RestoreState(const ApuFrameCounterState& state);

private:
    ApuFrameCounterState state_;

    Apu& apu_;
};