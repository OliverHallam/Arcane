#pragma once

#include <cstdint>

class ApuEnvelope
{
public:
    void Tick();

    void SetLoop(bool loop);
    void SetConstantVolume(bool constantVolume);
    void SetValue(uint8_t envelope);
    void Start();

    uint8_t Sample() const;

private:
    bool start_{};
    bool loop_{};

    bool constantVolume_{ true };
    uint8_t envelope_{};
    uint8_t decayLevel_{};

    uint8_t dividerCounter_{};
};