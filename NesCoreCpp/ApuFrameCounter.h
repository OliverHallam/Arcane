#pragma once

#include <cstdint>

class Apu;

class ApuFrameCounter
{
public:
    ApuFrameCounter(Apu& apu);

    void Tick();

private:
    uint_fast16_t cycleCount_;

    Apu& apu_;
};