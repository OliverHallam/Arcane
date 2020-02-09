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

    ApuFrameCounter frameCounter_;

    ApuPulse pulse1_;
    ApuPulse pulse2_;
    ApuTriangle triangle_;
    ApuNoise noise_;

    std::array<int16_t, SAMPLES_PER_FRAME> frameBuffer_;

    int currentSample_;
    int lastSampleCycle_;
    int sampleCounter_;

    bool odd_;

    uint32_t currentCycle_;
    uint32_t targetCycle_;

    uint8_t status_;
};