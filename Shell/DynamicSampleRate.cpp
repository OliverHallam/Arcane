#include "pch.h"

#include "DynamicSampleRate.h"

DynamicSampleRate::DynamicSampleRate(uint32_t sampleRate) :
    index_{ 0 },
    baseSampleRate_{ sampleRate },
    samplesWritten_{ },
    latency_{ static_cast<int32_t>(sampleRate) },
    targetLatency_{ static_cast<int32_t>(sampleRate) } // 1 frame
{
}

void DynamicSampleRate::OnFrame(uint64_t samplesWritten, uint64_t samplesPlayed)
{
    samplesWritten_ += samplesWritten;

    latency_ = static_cast<int32_t>(samplesWritten_ - samplesPlayed - targetLatency_);
}

void DynamicSampleRate::Reset()
{
    samplesWritten_ = 0;
    latency_ = targetLatency_;
}

uint32_t DynamicSampleRate::SampleRate() const
{
    auto drift = std::clamp(latency_ - targetLatency_, -targetLatency_, targetLatency_) / 50;
    return baseSampleRate_ - drift;
}

uint32_t DynamicSampleRate::TargetLatency() const
{
    return targetLatency_;
}
