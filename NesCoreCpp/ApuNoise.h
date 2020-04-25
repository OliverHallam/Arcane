#pragma once

#include "ApuEnvelope.h"
#include "ApuLengthCounter.h"

#include <cstdint>

class ApuNoise
{
public:
    ApuNoise();

    void Enabled(bool enabled);
    bool IsEnabled() const;

    void Write(uint16_t address, uint8_t value);

    void Run(uint32_t cycles);

    void TickQuarterFrame();
    void TickHalfFrame();

    int8_t Sample() const;

private:
    void StepSequencer();

    static uint_fast16_t LookupPeriod(uint8_t period);

    bool GetSequenceOutput() const;

    uint_fast16_t modeShift_;
    uint_fast16_t period_;
    uint32_t period2_;

    int32_t timer_;
    uint_fast16_t shifter_;

    ApuEnvelope envelope_;
    ApuLengthCounter lengthCounter_;
};