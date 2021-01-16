#include "Apu.h"
#include "Bus.h"

#include <cassert>

Apu::Apu(Bus& bus, uint32_t samplesPerFrame) :
    bus_(bus),
    frameCounter_{ *this },
    backBuffer_{ new int16_t[samplesPerFrame * 3ULL / 2] },
    sampleBuffer_{ new int16_t[samplesPerFrame * 3ULL / 2] },
    samplesPerFrame_(samplesPerFrame),
    pulse1_{ true },
    pulse2_{ false },
    dmc_{*this}
{
    // account for the fact we start off after VBlank
    state_.CurrentSample = (21 * samplesPerFrame) / 261;
    state_.SampleCycle = GetSampleCycle(state_.CurrentSample + 1);
    auto currentCycle = (21 * 341 / 3);
    bus_.Schedule(state_.SampleCycle - currentCycle, SyncEvent::ApuSample);
    bus_.Schedule(7457, SyncEvent::ApuFrameCounter);
}

void Apu::SetSamplesPerFrame(uint32_t samplesPerFrame)
{
    samplesPerFrame_ = samplesPerFrame;
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
    assert(state_.CurrentSample == samplesPerFrame_);

    std::swap(sampleBuffer_, backBuffer_);

    state_.CurrentSample = 0;
    state_.SampleCycle = 0;
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
        if (bus_.CpuCycleCount() & 1)
            nextTick++;

        bus_.Deschedule(SyncEvent::ApuFrameCounter);
        bus_.Schedule(nextTick, SyncEvent::ApuFrameCounter);
        break;
    }
}

uint8_t Apu::Read(uint16_t address)
{
    if (address == 0x4015)
    {
        Sync();

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

        if (state_.FrameCounterInterrupt)
            status |= 0x40;

        if (state_.DmcInterrupt)
            status |= 0x80;

        state_.FrameCounterInterrupt = false;
        bus_.SetAudioIrq(state_.DmcInterrupt);

        return status;
    }

    return 0;
}

void Apu::EnableMMC5(bool enabled)
{
    mmc5enabled_ = enabled;
}

void Apu::WriteMMC5(uint16_t address, uint8_t value)
{
    // TODO
}

uint32_t Apu::SamplesPerFrame() const
{
    return samplesPerFrame_;
}

const int16_t* Apu::Samples() const
{
    return sampleBuffer_.get();
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
    state_.FrameCounterInterrupt = interrupt;
    bus_.SetAudioIrq(state_.FrameCounterInterrupt || state_.DmcInterrupt);
}

void Apu::SetDmcInterrupt(bool interrupt)
{
    state_.DmcInterrupt = interrupt;
    bus_.SetAudioIrq(state_.FrameCounterInterrupt || state_.DmcInterrupt);
}

void Apu::Sync()
{
    auto cycle = bus_.CpuCycleCount();
    auto pendingCycles = cycle - state_.LastSyncCycle;

    pulse1_.Run(pendingCycles);
    pulse2_.Run(pendingCycles);
    triangle_.Run(pendingCycles);
    noise_.Run(pendingCycles);
    dmc_.Run(pendingCycles);

    state_.LastSyncCycle = cycle;
}

void Apu::ActivateFrameCounter()
{
    auto cycles = frameCounter_.Activate();
    bus_.Schedule(cycles, SyncEvent::ApuFrameCounter);
}

void Apu::CaptureState(ApuState* state) const
{
    frameCounter_.CaptureState(&state->FrameCounter);
    pulse1_.CaptureState(&state->Pulse1);
    pulse2_.CaptureState(&state->Pulse2);
    triangle_.CaptureState(&state->Triangle);
    noise_.CaptureState(&state->Noise);
    dmc_.CaptureState(&state->Dmc);

    state->Core = state_;

    if (!state->BackBuffer)
        state->BackBuffer.reset(new int16_t[samplesPerFrame_]);

    std::copy(&backBuffer_[0], &backBuffer_[0] + samplesPerFrame_, &state->BackBuffer[0]);
}

void Apu::RestoreState(const ApuState& state)
{
    frameCounter_.RestoreState(state.FrameCounter);
    pulse1_.RestoreState(state.Pulse1);
    pulse2_.RestoreState(state.Pulse2);
    triangle_.RestoreState(state.Triangle);
    noise_.RestoreState(state.Noise);
    dmc_.RestoreState(state.Dmc);

    state_ = state.Core;

    std::copy(&state.BackBuffer[0], &state.BackBuffer[0] + samplesPerFrame_, &backBuffer_[0]);
}

void Apu::Sample()
{
    Sync();

    assert(state_.CurrentSample < samplesPerFrame_);

    backBuffer_[state_.CurrentSample] =
        (pulse1_.Sample() << 5) +
        (pulse2_.Sample() << 5) +
        (triangle_.Sample() << 5) +
        (noise_.Sample() << 5) +
        (dmc_.Sample() << 5);

    state_.CurrentSample++;
    if (state_.CurrentSample < samplesPerFrame_)
    {
        // sometimes the PPU skips a cycle - because we are locking our 60Hz output to the PPU we'll need to slightly
        // abbreviate the last sample in that case.  For that reason, we let the PPU trigger the last cycle.
        auto nextSampleCycle = GetSampleCycle(state_.CurrentSample);
        bus_.Schedule(nextSampleCycle - state_.SampleCycle, SyncEvent::ApuSample);
        state_.SampleCycle = nextSampleCycle;
    }
}

uint32_t Apu::GetSampleCycle(uint32_t sample) const
{
    return (29781 * sample / samplesPerFrame_);
}
