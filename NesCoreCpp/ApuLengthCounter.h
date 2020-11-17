#pragma once

#include <cstdint>

#include "ApuLengthCounterState.h"

class ApuLengthCounter
{
public:
    void Tick();

    void SetHalt(bool halt);
    void SetLength(uint8_t length);
    void SetEnabled(bool enabled);

    bool IsEnabled() const;

    void CaptureState(ApuLengthCounterState* state) const;
    void RestoreState(const ApuLengthCounterState& state);

private:
    uint8_t GetLinearLength(uint8_t length);

    ApuLengthCounterState state_;
};