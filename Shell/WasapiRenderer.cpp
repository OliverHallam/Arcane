#include "pch.h"

#include "WasapiRenderer.h"


bool WasapiRenderer::Initialize()
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    auto deviceEnumerator = winrt::create_instance<IMMDeviceEnumerator>(__uuidof(MMDeviceEnumerator));

    auto hr = deviceEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eMultimedia, device_.put());
    winrt::check_hresult(hr);

    hr = device_->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, client_.put_void());
    winrt::check_hresult(hr);

    WAVEFORMATEX* mixFormat;
    hr = client_->GetMixFormat(&mixFormat);
    winrt::check_hresult(hr);
    sampleRate_ = mixFormat->nSamplesPerSec;

    // sanity check - all real life sample rates are.
    if (sampleRate_ % 60 != 0)
        return false;

    WAVEFORMATEX desiredFormat;
    ZeroMemory(&desiredFormat, sizeof(desiredFormat));
    desiredFormat.wFormatTag = WAVE_FORMAT_PCM; // TODO: use floats?
    desiredFormat.nChannels = 1;
    desiredFormat.nSamplesPerSec = sampleRate_;
    desiredFormat.nAvgBytesPerSec = sampleRate_ * sizeof(uint16_t);
    desiredFormat.nBlockAlign = sizeof(uint16_t);
    desiredFormat.wBitsPerSample = 16;
    desiredFormat.cbSize = 0;

    WAVEFORMATEX* closestFormat;
    hr = client_->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, &desiredFormat, &closestFormat);
    winrt::check_hresult(hr);
    if (hr != S_OK)
        return false;

    // we write one frame while the previous one is playing - so we'll store a buffer big enough for two
    auto samplesPerFrame = sampleRate_ / 60;
    auto bufferDuration = (uint64_t)samplesPerFrame * 10000 * 2;

    hr = client_->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, bufferDuration, 0, &desiredFormat, NULL);
    winrt::check_hresult(hr);

    return true;
}

uint32_t WasapiRenderer::SampleRate()
{
    return sampleRate_;
}
