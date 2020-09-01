#pragma once

#include <cstdint>

class Apu;

class ApuFrameCounter
{
public:
    ApuFrameCounter(Apu& apu);

    void EnableInterrupt(bool enable);
    uint32_t SetMode(uint8_t mode);

    uint32_t Activate();

private:
    uint_fast8_t phase_;

    uint8_t mode_{};

    bool enableInterrupt_{};

    Apu& apu_;
};