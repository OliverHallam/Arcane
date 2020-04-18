#pragma once

#include <mmdeviceapi.h>
#include <Audioclient.h>

class WasapiRenderer
{
public:
    bool Initialize();

    uint32_t SampleRate();

private:
    winrt::com_ptr<IMMDevice> device_;
    winrt::com_ptr<IAudioClient> client_;

    uint32_t sampleRate_{ 0 };
};