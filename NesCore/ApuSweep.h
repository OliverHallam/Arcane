#pragma once

#include "ApuSweepState.h"
#include <cstdint>

class ApuSweep
{
public:
    ApuSweep(bool pulse1);

    void SetPeriodHigh(uint8_t value);
    void SetPeriodLow(uint8_t value);
    void SetSweep(uint8_t value);

    void Tick();

    uint16_t Period() const;

    bool IsOutputEnabled() const;

    void CaptureState(ApuSweepState* state) const;
    void RestoreState(const ApuSweepState& state);

private:
    void UpdateTargetPeriod();

    uint16_t negatedDeltaOffset_{};

    ApuSweepState state_;
};