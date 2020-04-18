#pragma once

#include <mmdeviceapi.h>
#include <Audioclient.h>

class WasapiRenderer
{
public:
    bool Initialize();

    uint32_t SampleRate();

    void WriteSamples(const int16_t* samples, int sampleCount);

private:
    winrt::com_ptr<IMMDevice> device_;
    winrt::com_ptr<IAudioClient> client_;
    winrt::com_ptr<IAudioRenderClient> audioRenderClient_;

    uint32_t sampleRate_{ 0 };
    uint32_t bufferSize_{ 0 };
};