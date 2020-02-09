#pragma once

#include "ApuEnvelope.h"
#include "ApuLengthCounter.h"

#include <cstdint>

class ApuNoise
{
public:
    ApuNoise();

    void Enable(bool enabled);
    void Write(uint16_t address, uint8_t value);

    void Tick();
    void TickQuarterFrame();
    void TickHalfFrame();

    int8_t Sample();

private:
    void StepSequencer();

    uint_fast16_t LookupPeriod(uint8_t period);

    bool GetSequenceOutput();

    uint_fast16_t modeShift_;
    uint_fast16_t period_;

    uint_fast16_t timer_;
    uint_fast16_t shifter_;

    ApuEnvelope envelope_;
    ApuLengthCounter lengthCounter_;
};