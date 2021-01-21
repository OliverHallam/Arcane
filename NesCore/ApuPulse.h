#pragma once

#include "ApuEnvelope.h"
#include "ApuLengthCounter.h"
#include "ApuSweep.h"
#include "ApuPulseCoreState.h"
#include "ApuPulseState.h"

#include <cstdint>

class ApuPulse
{
public:
    ApuPulse(bool pulse1);

    void Enabled(bool enabled);
    bool IsEnabled() const;

    void Write(uint16_t address, uint8_t value);

    void Run(uint32_t cycles);

    void TickQuarterFrame();
    void TickHalfFrame();

    int8_t Sample() const;

    void CaptureState(ApuPulseState* state) const;
    void RestoreState(const ApuPulseState& state);

private:
    static uint8_t GetDutyLookup(uint8_t duty);
    bool GetSequenceOutput() const;

    ApuEnvelope envelope_{};
    ApuLengthCounter lengthCounter_{};
    ApuSweep sweep_;

    ApuPulseCoreState state_;
};