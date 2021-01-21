#pragma once

#include "ApuEnvelope.h"
#include "ApuLengthCounter.h"
#include "ApuNoiseCoreState.h"
#include "ApuNoiseState.h"

#include <cstdint>

class ApuNoise
{
public:
    ApuNoise();

    void Enable(bool enabled);
    bool IsEnabled() const;

    void Write(uint16_t address, uint8_t value);

    void Run(uint32_t cycles);

    void TickQuarterFrame();
    void TickHalfFrame();

    int8_t Sample() const;

    void CaptureState(ApuNoiseState* state) const;
    void RestoreState(const ApuNoiseState& state);

private:
    void StepSequencer();

    static uint_fast16_t LookupPeriod(uint8_t period);

    bool GetSequenceOutput() const;

    ApuEnvelope envelope_;
    ApuLengthCounter lengthCounter_;

    ApuNoiseCoreState state_;
};