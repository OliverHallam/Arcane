#include "Apu.h"
#include "Bus.h"

#include <cassert>

Apu::Apu(Bus& bus, uint32_t samplesPerFrame) :
    bus_(bus),
    frameCounter_{ *this },
    frameBuffer_{ new int16_t[samplesPerFrame] },
    currentSample_(0),
    samplesPerFrame_(samplesPerFrame),
    lastSampleCycle_(29780 / samplesPerFrame),
    lastSyncCycle_{ 0 },
    pulse1_{ true },
    pulse2_{ false },
    dmc_{*this}
{
    // account for the fact we start off after VBlank
    currentSample_ = (21 * samplesPerFrame) / 261;
    lastSampleCycle_ = ((currentSample_ + 1) * 29781) / samplesPerFrame;
    auto currentCycle = (21 * 341 / 3);
    bus_.Schedule(lastSampleCycle_ - currentCycle, SyncEvent::ApuSample);
    bus_.Schedule(7457, SyncEvent::ApuFrameCounter);
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
    assert(currentSample_ == samplesPerFrame_);

    currentSample_ = 0;
    lastSampleCycle_ = 0;
    Sample();
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
        pulse1_.Write(address, value);
        break;

    case 0x4004:
    case 0x4005:
    case 0x4006:
    case 0x4007:
        pulse2_.Write(address, value);
        break;

    case 0x4008:
    case 0x4009:
    case 0x400a:
    case 0x400b:
        triangle_.Write(address, value);
        break;

    case 0x400c:
    case 0x400d:
    case 0x400e:
    case 0x400f:
        noise_.Write(address, value);
        break;

    case 0x4010:
    case 0x4011:
    case 0x4012:
    case 0x4013:
    {
        dmc_.Write(address, value);
        break;
    }

    case 0x4015:
        pulse1_.Enabled(value & 0x01);
        pulse2_.Enabled(value & 0x02);
        triangle_.Enable(value & 0x04);
        noise_.Enable(value & 0x08);
        dmc_.Enable(value & 0x10);
        break;

    case 0x4017:
        frameCounter_.EnableInterrupt((value & 0x40) == 0);
        auto nextTick = frameCounter_.SetMode(value >> 7);

        // sync to audio clock
        if (bus_.CycleCount() & 1)
            nextTick++;

        bus_.RescheduleFrameCounter(nextTick);
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

        if (dmc_.IsEnabled())
            status |= 0x10;

        if (frameCounterInterrupt_)
            status |= 0x40;

        if (dmcInterrupt_)
            status |= 0x80;

        frameCounterInterrupt_ = false;
        bus_.SetAudioIrq(dmcInterrupt_);

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

void Apu::ScheduleDmc(uint32_t cycles)
{
    bus_.Schedule(cycles, SyncEvent::ApuSync);
}

void Apu::RequestDmcByte(uint16_t address)
{
    bus_.BeginDmcDma(address);
}

void Apu::SetDmcBuffer(uint8_t value)
{
    dmc_.SetBuffer(value);
}

void Apu::SetFrameCounterInterrupt(bool interrupt)
{
    frameCounterInterrupt_ = interrupt;
    bus_.SetAudioIrq(frameCounterInterrupt_ || dmcInterrupt_);
}

void Apu::SetDmcInterrupt(bool interrupt)
{
    dmcInterrupt_ = interrupt;
    bus_.SetCartIrq(frameCounterInterrupt_ || dmcInterrupt_);
}

void Apu::Sync()
{
    auto cycle = bus_.CycleCount();
    auto pendingCycles = cycle - lastSyncCycle_;

    pulse1_.Run(pendingCycles);
    pulse2_.Run(pendingCycles);
    triangle_.Run(pendingCycles);
    noise_.Run(pendingCycles);
    dmc_.Run(pendingCycles);

    lastSyncCycle_ = cycle;
}

void Apu::ActivateFrameCounter()
{
    auto cycles = frameCounter_.Activate();
    bus_.Schedule(cycles, SyncEvent::ApuFrameCounter);
}

void Apu::Sample()
{
    Sync();

    assert(currentSample_ < samplesPerFrame_);

    frameBuffer_[currentSample_++] =
        (pulse1_.Sample() << 7) +
        (pulse2_.Sample() << 7) +
        (triangle_.Sample() << 7) +
        (noise_.Sample() << 7) +
        (dmc_.Sample() << 7);

    // we'll let the PPU kick start the next frame, due to the fact that the cycles don't quite line up
    if (currentSample_ != samplesPerFrame_)
    {
        auto nextSampleCycle = (29781 * currentSample_) / samplesPerFrame_;
        bus_.Schedule(nextSampleCycle - lastSampleCycle_, SyncEvent::ApuSample);
        lastSampleCycle_ = nextSampleCycle;
    }
}
