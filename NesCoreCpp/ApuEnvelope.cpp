#include "ApuEnvelope.h"

void ApuEnvelope::Tick()
{
    if (start_)
    {
        start_ = false;
        decayLevel_ = 15;
        dividerCounter_ = envelope_;
    }
    else
    {
        if (dividerCounter_)
        {
            dividerCounter_--;
        }
        else
        {
            dividerCounter_ = envelope_;
            if (decayLevel_)
            {
                decayLevel_--;
            }
            else
            {
                if (loop_)
                    decayLevel_ = 15;
            }
        }
    }

}

void ApuEnvelope::SetConstantVolume(bool constantVolume)
{
    constantVolume_ = constantVolume;
}

void ApuEnvelope::SetEnvelope(uint8_t envelope)
{
    envelope_ = envelope;
}

void ApuEnvelope::Start()
{
    start_ = true;
}

uint8_t ApuEnvelope::Sample()
{
    return constantVolume_ ? envelope_ : decayLevel_;
}
