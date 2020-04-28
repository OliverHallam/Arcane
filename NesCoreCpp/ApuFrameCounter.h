#pragma once

#include <cstdint>

class Apu;

class ApuFrameCounter
{
public:
    ApuFrameCounter(Apu& apu);

    void EnableInterrupt(bool enable);
    void SetMode(uint8_t mode);

    void Tick();

private:
    void Activate();

    uint_fast16_t counter_;
    uint_fast8_t phase_;

    uint8_t mode_{};

    bool enableInterrupt_{};

    Apu& apu_;
};