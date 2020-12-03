#include "pch.h"

#include "Host.h"

Host::Host()
    : sampleRate_{ 0 },
    running_ { false }
{
}

void Host::SetSampleRate(uint32_t sampleRate)
{
    sampleRate_ = sampleRate;
}

void Host::Load(std::unique_ptr<Cart> cartridge)
{
    if (!system_)
        system_ = std::make_unique<NesSystem>(sampleRate_);

    system_->InsertCart(std::move(cartridge));
    system_->Reset();

    system_->CaptureState(&state_);
}

void Host::Unload()
{
    system_.release();
}

void Host::Start()
{
    if (system_)
    {
        running_ = true;
        step_ = false;
    }
}

void Host::Stop()
{
    running_ = false;
}

void Host::Step()
{
    running_ = true;
    step_ = true;
}

bool Host::Loaded() const
{
    return !!system_;
}

bool Host::Running() const
{
    return running_;
}

void Host::RunFrame()
{
    system_->RunFrame();

    if (step_)
        running_ = false;
}

const uint32_t* Host::PixelData() const
{
    return system_->Display().Buffer();
}

uint32_t Host::RefreshRate() const
{
    return 60;
}

const int16_t* Host::AudioSamples() const
{
    return system_->Apu().Samples();
}

const uint32_t Host::SamplesPerFrame() const
{
    return system_->Apu().SamplesPerFrame();
}

void Host::SetSamplesPerFrame(uint32_t samples)
{
    system_->Apu().SetSamplesPerFrame(samples);
}

void Host::Snapshot()
{
    system_->CaptureState(&state_);
}

void Host::Restore()
{
    system_->RestoreState(state_);
}

void Host::Up(bool pressed)
{
    system_->Controller().Up(pressed);
}

void Host::Down(bool pressed)
{
    system_->Controller().Down(pressed);
}

void Host::Left(bool pressed)
{
    system_->Controller().Left(pressed);
}

void Host::Right(bool pressed)
{
    system_->Controller().Right(pressed);
}

void Host::A(bool pressed)
{
    system_->Controller().A(pressed);
}

void Host::B(bool pressed)
{
    system_->Controller().B(pressed);
}

void Host::Select(bool pressed)
{
    system_->Controller().Select(pressed);
}

void Host::Start(bool pressed)
{
    system_->Controller().Start(pressed);
}
