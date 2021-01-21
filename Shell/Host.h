#pragma once

#include <memory>
#include "..\NesCore\NesSystem.h"
#include "..\NesCore\SystemState.h"

#include "RewindBuffer.h"

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

    void EnableRewind();
    void DisableRewind();

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

    std::unique_ptr<RewindBuffer> rewindBuffer_;

    bool running_;
    bool step_;

    bool wasRewindPressed_;
    bool rewind_;

    SystemState state_;
};