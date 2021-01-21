#include "ApuEnvelope.h"

void ApuEnvelope::Tick()
{
    if (state_.Start)
    {
        state_.Start = false;
        state_.DecayLevel = 15;
        state_.DividerCounter = state_.Envelope;
    }
    else
    {
        if (state_.DividerCounter)
        {
            state_.DividerCounter--;
        }
        else
        {
            state_.DividerCounter = state_.Envelope;
            if (state_.DecayLevel)
            {
                state_.DecayLevel--;
            }
            else
            {
                if (state_.Loop)
                    state_.DecayLevel = 15;
            }
        }
    }
}

void ApuEnvelope::SetLoop(bool loop)
{
    state_.Loop = loop;
}

void ApuEnvelope::SetConstantVolume(bool constantVolume)
{
    state_.ConstantVolume = constantVolume;
}

void ApuEnvelope::SetValue(uint8_t envelope)
{
    state_.Envelope = envelope;
}

void ApuEnvelope::Start()
{
    state_.Start = true;
}

uint8_t ApuEnvelope::Sample() const
{
    return state_.ConstantVolume ? state_.Envelope : state_.DecayLevel;
}

void ApuEnvelope::CaptureState(ApuEnvelopeState* state) const
{
    *state = state_;
}

void ApuEnvelope::RestoreState(const ApuEnvelopeState& state)
{
    state_ = state;
}
