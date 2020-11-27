#pragma once

#include <mmdeviceapi.h>
#include <Audioclient.h>

class WasapiRenderer
{
public:
    void Initialize();

    uint32_t SampleRate();

    void WriteSamples(const int16_t* samples, int sampleCount);

    uint64_t GetPosition();

private:
    winrt::com_ptr<IMMDevice> device_;
    winrt::com_ptr<IAudioClient> client_;
    winrt::com_ptr<IAudioRenderClient> audioRenderClient_;
    winrt::com_ptr<IAudioClock> audioClock_;

    uint32_t bufferSize_{ 0 };

    uint32_t sampleRate_{ 0 };
    uint32_t outputChannels_{ 0 };

    uint64_t frequency_{};

    DWORD channelMask_{ 0 };
};