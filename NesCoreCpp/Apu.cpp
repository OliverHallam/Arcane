#include "Apu.h"

#include <assert.h>

Apu::Apu(uint32_t samplesPerFrame) :
    frameCounter_{*this},
    frameBuffer_{new int16_t[samplesPerFrame]},
    currentSample_(0),
    samplesPerFrame_(samplesPerFrame),
    lastSampleCycle_(29780 / samplesPerFrame),
    sampleCounter_(29780 / samplesPerFrame),
    pulse1_{true},
    pulse2_{false}
{
}

void Apu::Tick()
{
    frameCounter_.Tick();

    pendingCycles_++;

    if (!--sampleCounter_)
    {
        Sample();
    }
}

void Apu::QuarterFrame()
{
    Sync();

    pulse1_.TickQuarterFrame();
    pulse2_.TickQuarterFrame();
    triangle_.TickQuarterFrame();
    noise_.TickQuarterFrame();
}

void Apu::HalfFrame()
{
    Sync();

    pulse1_.TickHalfFrame();
    pulse2_.TickHalfFrame();
    triangle_.TickHalfFrame();
    noise_.TickHalfFrame();
}

void Apu::SyncFrame()
{
    Sync();

    assert(currentSample_ == samplesPerFrame_);

    currentSample_ = 0;
    lastSampleCycle_ = sampleCounter_ = 29780 / samplesPerFrame_;
}

void Apu::Write(uint16_t address, uint8_t value)
{
    Sync();

    switch (address)
    {
    case 0x4000:
    case 0x4001:
    case 0x4002:
    case 0x4003:
        pulse1_.Write(address & 0x0003, value);
        break;

    case 0x4004:
    case 0x4005:
    case 0x4006:
    case 0x4007:
        pulse2_.Write(address & 0x0003, value);
        break;

    case 0x4008:
    case 0x4009:
    case 0x400a:
    case 0x400b:
        triangle_.Write(address & 0x0003, value);
        break;

    case 0x400c:
    case 0x400d:
    case 0x400e:
    case 0x400f:
        noise_.Write(address & 0x0003, value);
        break;

    case 0x4015:
        if (!(value & 0x01))
            pulse1_.Disable();

        if (!(value & 0x02))
            pulse2_.Disable();

        if (!(value & 0x04))
            triangle_.Disable();

        if (!(value & 0x08))
            noise_.Disable();

        break;

    case 0x4017:
        // TODO: interrupt mask
        frameCounter_.SetMode(value >> 7);
        break;
    }
}

uint8_t Apu::Read(uint16_t address)
{
    if (address == 0x4015)
    {
        uint8_t status = 0;
        if (pulse1_.IsEnabled())
            status |= 0x01;

        if (pulse2_.IsEnabled())
            status |= 0x02;

        if (triangle_.IsEnabled())
            status |= 0x04;

        if (noise_.IsEnabled())
            status |= 0x08;

        return status;
    }

    return 0;
}

uint32_t Apu::SamplesPerFrame() const
{
    return samplesPerFrame_;
}

const int16_t* Apu::Samples() const
{
    return frameBuffer_.get();
}

void Apu::Sync()
{
    pulse1_.Run(pendingCycles_);
    pulse2_.Run(pendingCycles_);
    triangle_.Run(pendingCycles_);
    noise_.Run(pendingCycles_);

    pendingCycles_ = 0;
}

void Apu::Sample()
{
    Sync();

    assert(currentSample_ < samplesPerFrame_);

    frameBuffer_[currentSample_++] =
        (pulse1_.Sample() << 7) +
        (pulse2_.Sample() << 7) +
        (triangle_.Sample() << 7) +
        (noise_.Sample() << 7);
    auto nextSampleCycle = (29780 * (currentSample_ + 1)) / samplesPerFrame_;
    sampleCounter_ = nextSampleCycle - lastSampleCycle_;
    lastSampleCycle_ = nextSampleCycle;
}
