#pragma once

#include "ApuEnvelope.h"
#include "ApuFrameCounter.h"
#include "ApuPulse.h"
#include "ApuNoise.h"

#include <array>
#include <cstdint>

class Apu
{
public:
    static const int SAMPLES_PER_FRAME = 735; // 44100 Hz

    Apu();

    void Tick();
    void QuarterFrame();
    void HalfFrame();

    void SyncFrame();

    void Write(uint16_t address, uint8_t value);

    const std::array<int16_t, SAMPLES_PER_FRAME>& Samples() const;

private:
    ApuFrameCounter frameCounter_;

    ApuPulse pulse1_;
    ApuPulse pulse2_;
    ApuNoise noise_;

    std::array<int16_t, SAMPLES_PER_FRAME> frameBuffer_;

    int currentSample_;
    int cycleCount_;
    int nextSampleCycle_;
};