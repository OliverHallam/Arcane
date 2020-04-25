#pragma once

#include "ApuEnvelope.h"
#include "ApuFrameCounter.h"
#include "ApuNoise.h"
#include "ApuPulse.h"
#include "ApuTriangle.h"

#include <memory>
#include <cstdint>

class Apu
{
public:
    Apu(uint32_t samplesPerFrame);

    void Tick();
    void QuarterFrame();
    void HalfFrame();

    void SyncFrame();

    void Write(uint16_t address, uint8_t value);
    uint8_t Read(uint16_t address);

    uint32_t SamplesPerFrame() const;
    const int16_t* Samples() const;

private:
    void Sync();
    void Sample();

    ApuFrameCounter frameCounter_;

    ApuPulse pulse1_;
    ApuPulse pulse2_;
    ApuTriangle triangle_;
    ApuNoise noise_;

    std::unique_ptr<int16_t[]> frameBuffer_;

    uint32_t samplesPerFrame_;
    uint32_t currentSample_;
    uint32_t lastSampleCycle_;
    uint32_t sampleCounter_;

    uint32_t pendingCycles_{};
};