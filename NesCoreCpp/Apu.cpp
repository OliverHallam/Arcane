#include "Apu.h"

Apu::Apu() :
    frameCounter_{*this},
    cycleCount_(0),
    currentSample_(0),
    nextSampleCycle_(29780 / SAMPLES_PER_FRAME)
{
}

void Apu::Tick()
{
    frameCounter_.Tick();

    cycleCount_++;
    if ((cycleCount_ & 1))
    {
        pulse1_.Tick();
        pulse2_.Tick();
    }

    if (cycleCount_ == nextSampleCycle_)
    {
        frameBuffer_[currentSample_++] = (pulse1_.Sample() << 8) + (pulse2_.Sample() << 8);
        nextSampleCycle_ = (29780 * (currentSample_ + 1)) / SAMPLES_PER_FRAME;
    }
}

void Apu::QuarterFrame()
{
    pulse1_.TickQuarterFrame();
    pulse2_.TickQuarterFrame();
}

void Apu::HalfFrame()
{
    pulse1_.TickHalfFrame();
    pulse2_.TickHalfFrame();
}

void Apu::SyncFrame()
{
    frameBuffer_.back() = pulse1_.Sample();
    currentSample_ = 0;
    cycleCount_ = 0;
    nextSampleCycle_ = 29780 / SAMPLES_PER_FRAME;
}

void Apu::Write(uint16_t address, uint8_t value)
{
    switch (address)
    {
    case 0x4000:
    case 0x4001:
    case 0x4002:
    case 0x4003:
        pulse1_.Write(address & 0x3, value);
        break;

    case 0x4004:
    case 0x4005:
    case 0x4006:
    case 0x4007:
        pulse2_.Write(address & 0x3, value);
        break;

    case 0x4015:
        pulse1_.Enable(value & 0x01);
        pulse2_.Enable(value & 0x02);
    }
}

const std::array<int16_t, Apu::SAMPLES_PER_FRAME>& Apu::Samples() const
{
    return frameBuffer_;
}
