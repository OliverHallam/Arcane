#pragma once

#include "ApuDmc.h"
#include "ApuFrameCounter.h"
#include "ApuNoise.h"
#include "ApuPulse.h"
#include "ApuState.h"
#include "ApuTriangle.h"

#include <memory>
#include <cstdint>

class Bus;
struct ApuState;

class Apu
{
public:
    Apu(Bus& bus, uint32_t samplesPerFrame);

    void SetSamplesPerFrame(uint32_t samplesPerFrame);

    void QuarterFrame();
    void HalfFrame();

    void SyncFrame();

    void Write(uint16_t address, uint8_t value);
    uint8_t Read(uint16_t address);

    void EnableMMC5(bool enabled);
    void WriteMMC5(uint16_t address, uint8_t value);

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

    void CaptureState(ApuState* state) const;
    void RestoreState(const ApuState& state);

private:
    uint32_t GetSampleCycle(uint32_t sample) const;

    Bus& bus_;

    ApuFrameCounter frameCounter_;

    ApuPulse pulse1_;
    ApuPulse pulse2_;
    ApuTriangle triangle_;
    ApuNoise noise_;
    ApuDmc dmc_;

    ApuCoreState state_;

    std::unique_ptr<int16_t[]> backBuffer_;
    std::unique_ptr<int16_t[]> sampleBuffer_;

    uint32_t samplesPerFrame_;

    bool mmc5enabled_;
};