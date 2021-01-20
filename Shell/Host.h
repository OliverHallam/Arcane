#pragma once

#include <memory>
#include "..\NesCoreCpp\NesSystem.h"
#include "..\NesCoreCpp\SystemState.h"

class Host
{
public:
    Host();

    void SetSampleRate(uint32_t sampleRate);

    void Load(std::unique_ptr<Cart> cartridge);
    void Unload();

    void Start();
    void Stop();
    void Step();
    void Restart();

    bool Loaded() const;
    bool Running() const;

    void RunFrame();

    const uint32_t* PixelData() const;
    uint32_t RefreshRate() const;

    const int16_t* AudioSamples() const;
    const uint32_t SamplesPerFrame() const;
    void SetSamplesPerFrame(uint32_t samples);

    void Snapshot();
    void Restore();

    void SetController1State(int32_t buttons);
    void SetController2State(int32_t buttons);

private:
    uint32_t sampleRate_;
    std::unique_ptr<NesSystem> system_;

    bool running_;
    bool step_;

    SystemState state_;
};