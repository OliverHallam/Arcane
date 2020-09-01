#pragma once

#include "ApuDmc.h"
#include "ApuFrameCounter.h"
#include "ApuNoise.h"
#include "ApuPulse.h"
#include "ApuTriangle.h"

#include <memory>
#include <cstdint>

class Bus;

class Apu
{
public:
    Apu(Bus& bus, uint32_t samplesPerFrame);

    void Tick();
    void QuarterFrame();
    void HalfFrame();

    void SyncFrame();

    void Write(uint16_t address, uint8_t value);
    uint8_t Read(uint16_t address);

    uint32_t SamplesPerFrame() const;
    const int16_t* Samples() const;

    void ScheduleDmc(uint32_t cycles);

    void RequestDmcByte(uint16_t address);
    void SetDmcBuffer(uint8_t value);

    void SetFrameCounterInterrupt(bool interrupt);
    void SetDmcInterrupt(bool interrupt);

    void Sample();
    void Sync();
    void ActivateFrameCounter();

private:

    Bus& bus_;

    ApuFrameCounter frameCounter_;

    ApuPulse pulse1_;
    ApuPulse pulse2_;
    ApuTriangle triangle_;
    ApuNoise noise_;
    ApuDmc dmc_;

    std::unique_ptr<int16_t[]> frameBuffer_;

    uint32_t samplesPerFrame_;
    uint32_t currentSample_;
    uint32_t lastSampleCycle_;

    uint32_t pendingCycles_{};

    bool dmcInterrupt_{};
    bool frameCounterInterrupt_{};
};