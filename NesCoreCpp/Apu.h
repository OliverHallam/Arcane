#pragma once

#include "ApuEnvelope.h"
#include "ApuFrameCounter.h"
#include "ApuNoise.h"
#include "ApuPulse.h"
#include "ApuTriangle.h"

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
    uint8_t Read(uint16_t address);

    const std::array<int16_t, SAMPLES_PER_FRAME>& Samples() const;

private:
    void Sync();
    void Sample();

    ApuFrameCounter frameCounter_;

    ApuPulse pulse1_;
    ApuPulse pulse2_;
    ApuTriangle triangle_;
    ApuNoise noise_;

    std::array<int16_t, SAMPLES_PER_FRAME> frameBuffer_;

    uint32_t currentSample_;
    uint32_t lastSampleCycle_;
    uint32_t sampleCounter_;

    uint32_t pendingCycles_{};

    uint8_t status_;
};