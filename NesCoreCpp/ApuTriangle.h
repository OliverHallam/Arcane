#pragma once

#include "ApuLengthCounter.h"
#include "ApuTriangleCoreState.h"
#include "ApuTriangleState.h"

class ApuTriangle
{
public:
    void Enable(bool enabled);
    bool IsEnabled() const;

    void Write(uint16_t address, uint8_t value);

    void Run(uint32_t cycles);

    void TickQuarterFrame();
    void TickHalfFrame();

    int8_t Sample() const;

    void CaptureState(ApuTriangleState* state) const;
    void RestoreState(const ApuTriangleState& state);

private:
    ApuLengthCounter lengthCounter_;

    ApuTriangleCoreState state_;
};