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

    WAVEFORMATEXTENSIBLE* mixFormat;
    hr = client_->GetMixFormat(reinterpret_cast<WAVEFORMATEX **>(&mixFormat));
    winrt::check_hresult(hr);

    sampleRate_ = mixFormat->Format.nSamplesPerSec;
    outputChannels_ = mixFormat->Format.nChannels;
    channelMask_ = mixFormat->dwChannelMask;

    CoTaskMemFree(mixFormat);

    // sanity check - all real life sample rates are.
    if (sampleRate_ % 60 != 0)
    {
        std::wstringstream ss;
        ss << L"Unsupported sample rate: " << sampleRate_ << "Hz";
        throw Error(ss.str());
    }

    WAVEFORMATEXTENSIBLE desiredFormat;
    ZeroMemory(&desiredFormat, sizeof(WAVEFORMATEXTENSIBLE));
    desiredFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE; // TODO: use floats?
    desiredFormat.Format.nChannels = outputChannels_;
    desiredFormat.Format.nSamplesPerSec = sampleRate_;
    desiredFormat.Format.nAvgBytesPerSec = sampleRate_ * outputChannels_ * sizeof(uint16_t);
    desiredFormat.Format.nBlockAlign = static_cast<WORD>(sizeof(uint16_t) * outputChannels_);
    desiredFormat.Format.wBitsPerSample = 16;
    desiredFormat.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
    desiredFormat.Samples.wValidBitsPerSample = 16;
    desiredFormat.dwChannelMask = channelMask_;
    desiredFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

    WAVEFORMATEXTENSIBLE* closestFormat;
    hr = client_->IsFormatSupported(
        AUDCLNT_SHAREMODE_SHARED,
        &desiredFormat.Format,
        reinterpret_cast<WAVEFORMATEX**>(&closestFormat));
    CoTaskMemFree(closestFormat);
    winrt::check_hresult(hr);
    if (hr != S_OK)
        throw Error(L"Failed to negotiate wave format");

    if (((channelMask_ & KSAUDIO_SPEAKER_STEREO) != KSAUDIO_SPEAKER_STEREO)
        && ((channelMask_ & SPEAKER_FRONT_CENTER) != SPEAKER_FRONT_CENTER))
        throw Error(L"Unsupported speaker configuration");

    // we write one frame while the previous one is playing - so we'll store a buffer big enough for two
    auto samplesPerFrame = sampleRate_ / 60;
    auto bufferDuration = (uint64_t)samplesPerFrame * 10000 * 3;

    hr = client_->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        bufferDuration,
        0,
        reinterpret_cast<const WAVEFORMATEX*>(&desiredFormat),
        NULL);
    winrt::check_hresult(hr);

    hr = client_->GetBufferSize(&bufferSize_);
    winrt::check_hresult(hr);

    hr = client_->GetService(__uuidof(IAudioRenderClient), audioRenderClient_.put_void());
    winrt::check_hresult(hr);

    // clear the buffer
    BYTE* data;
    hr = audioRenderClient_->GetBuffer(samplesPerFrame * 2, &data);
    winrt::check_hresult(hr);

    audioRenderClient_->ReleaseBuffer(samplesPerFrame * 2, AUDCLNT_BUFFERFLAGS_SILENT);

    hr = client_->GetService(__uuidof(IAudioClock), audioClock_.put_void());
    winrt::check_hresult(hr);

    hr = client_->Start();
    winrt::check_hresult(hr);

    hr = audioClock_->GetFrequency(&frequency_);
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

    auto inSampleMono = samples;

    if ((channelMask_ & SPEAKER_FRONT_CENTER) != 0)
    {
        auto centreOffset = (channelMask_ & KSAUDIO_SPEAKER_STEREO) == KSAUDIO_SPEAKER_STEREO ? 2 : 0;

        ZeroMemory(data, static_cast<size_t>(sampleCount) * sizeof(uint16_t) * outputChannels_);

        auto outSample = reinterpret_cast<uint16_t *>(data) + centreOffset;
        for (auto i = sampleCount; i >= 0; i--)
        {
            auto monoSample = *inSampleMono++;
            *outSample = monoSample;
            outSample += outputChannels_;
        }
    }
    else
    {
        ZeroMemory(data, static_cast<size_t>(sampleCount) * sizeof(uint16_t) * outputChannels_);

        auto outSample = reinterpret_cast<uint16_t*>(data);
        for (auto i = sampleCount; i >= 0; i--)
        {
            auto monoSample = *inSampleMono++;
            *outSample = monoSample;
            *(outSample + 1) = monoSample;
            outSample += outputChannels_;
        }
    }

    audioRenderClient_->ReleaseBuffer(sampleCount, 0);
}

uint64_t WasapiRenderer::GetPosition()
{
    UINT64 position, qpcPosition;
    audioClock_->GetPosition(&position, &qpcPosition);

    return position * sampleRate_ / frequency_;
}
