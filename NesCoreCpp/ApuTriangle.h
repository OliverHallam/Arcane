#pragma once

#include "ApuLengthCounter.h"

class ApuTriangle
{
public:
    void Enable(bool enabled);
    void Write(uint16_t address, uint8_t value);

    void Tick();
    void TickQuarterFrame();
    void TickHalfFrame();

    int8_t Sample();

private:
    ApuLengthCounter lengthCounter_;

    uint16_t period_{};
    uint16_t timer_{};

    int waveformCycle_{};

    uint16_t linearCounter_{};
    bool linearCounterReload_{};
    uint16_t linearCounterReloadValue_{};
    bool control_{};
};