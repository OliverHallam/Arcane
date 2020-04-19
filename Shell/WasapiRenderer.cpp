#include "pch.h"

#include "Error.h"
#include "WasapiRenderer.h"

#include <sstream>

void WasapiRenderer::Initialize()
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
    {
        std::wstringstream ss;
        ss << L"Unsupported sample rate: " << sampleRate_ << "Hz";
        throw Error(ss.str());
    }

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
        throw Error(L"Failed to negotiate wave format");

    // we write one frame while the previous one is playing - so we'll store a buffer big enough for two
    auto samplesPerFrame = sampleRate_ / 60;
    auto bufferDuration = (uint64_t)samplesPerFrame * 10000 * 2;

    hr = client_->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, bufferDuration, 0, &desiredFormat, NULL);
    winrt::check_hresult(hr);

    hr = client_->GetBufferSize(&bufferSize_);
    winrt::check_hresult(hr);

    hr = client_->GetService(__uuidof(IAudioRenderClient), audioRenderClient_.put_void());
    winrt::check_hresult(hr);

    // clear the buffer
    BYTE* data;
    hr = audioRenderClient_->GetBuffer(samplesPerFrame, &data);
    winrt::check_hresult(hr);

    audioRenderClient_->ReleaseBuffer(samplesPerFrame, AUDCLNT_BUFFERFLAGS_SILENT);

    hr = client_->Start();
    winrt::check_hresult(hr);
}

uint32_t WasapiRenderer::SampleRate()
{
    return sampleRate_;
}

void WasapiRenderer::WriteSamples(const int16_t* samples, int sampleCount)
{
    BYTE* data;
    auto hr = audioRenderClient_->GetBuffer(sampleCount, &data);
    winrt::check_hresult(hr);

    memcpy(data, samples, sampleCount * sizeof(uint16_t));

    audioRenderClient_->ReleaseBuffer(sampleCount, 0);
}
