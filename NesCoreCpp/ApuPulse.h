#pragma once

#include "ApuEnvelope.h"
#include "ApuLengthCounter.h"
#include "ApuSweep.h"

#include <cstdint>

class ApuPulse
{
public:
    ApuPulse();

    void Enable(bool enabled);
    void Write(uint8_t address, uint8_t value);

    void Tick();
    void TickQuarterFrame();
    void TickHalfFrame();

    int8_t Sample();

private:
    void StepSequencer();

    uint8_t GetDutyLookup(uint8_t duty);
    bool GetSequenceOutput();

    uint16_t timer_{};
    uint8_t sequence_{};
    uint8_t dutyLookup_{};

    ApuEnvelope envelope_{};
    ApuLengthCounter lengthCounter_{};
    ApuSweep sweep_{};
};