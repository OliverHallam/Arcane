#pragma once

#include <cstdint>

#include "ApuEnvelopeState.h"

class ApuEnvelope
{
public:
    void Tick();

    void SetLoop(bool loop);
    void SetConstantVolume(bool constantVolume);
    void SetValue(uint8_t envelope);
    void Start();

    uint8_t Sample() const;

    void CaptureState(ApuEnvelopeState* state) const;
    void RestoreState(const ApuEnvelopeState& state);

private:
    ApuEnvelopeState state_;
};