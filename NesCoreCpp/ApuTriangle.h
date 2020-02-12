#pragma once

#include "ApuLengthCounter.h"

class ApuTriangle
{
public:
    void Enable(bool enabled);
    void Write(uint16_t address, uint8_t value);

    void Run(uint32_t cycles);

    void TickQuarterFrame();
    void TickHalfFrame();

    int8_t Sample();

private:
    ApuLengthCounter lengthCounter_;

    uint32_t period_{};
    uint32_t period2_{2};
    int32_t timer_{};

    int waveformCycle_{};

    uint32_t linearCounter_{};
    bool linearCounterReload_{};
    uint32_t linearCounterReloadValue_{};
    bool control_{};
};