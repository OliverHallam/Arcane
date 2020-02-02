#pragma once

#include <cstdint>

class Apu;

class ApuFrameCounter
{
public:
    ApuFrameCounter(Apu& apu);

    void SetMode(uint8_t mode);

    void Tick();

    uint8_t Mode();

private:
    uint_fast16_t cycleCount_{};

    uint8_t mode_{};

    Apu& apu_;
};