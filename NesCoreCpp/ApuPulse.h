#pragma once

#include "ApuEnvelope.h"
#include "ApuLengthCounter.h"
#include "ApuSweep.h"

#include <cstdint>

class ApuPulse
{
public:
    ApuPulse(bool pulse1);

    void Enabled(bool enabled);
    bool IsEnabled() const;

    void Write(uint8_t address, uint8_t value);

    void Run(uint32_t cycles);

    void TickQuarterFrame();
    void TickHalfFrame();

    int8_t Sample() const;

private:
    static uint8_t GetDutyLookup(uint8_t duty);
    bool GetSequenceOutput() const;

    int32_t timer_{};
    uint32_t sequence_{};
    uint32_t dutyLookup_{};

    ApuEnvelope envelope_{};
    ApuLengthCounter lengthCounter_{};
    ApuSweep sweep_;
};