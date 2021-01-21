#pragma once

struct ApuEnvelopeState
{
    bool Start{};
    bool Loop{};

    bool ConstantVolume{ true };
    uint8_t Envelope{};
    uint8_t DecayLevel{};

    uint8_t DividerCounter{};
};