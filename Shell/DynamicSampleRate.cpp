#include "pch.h"

#include "DynamicSampleRate.h"

DynamicSampleRate::DynamicSampleRate(uint32_t sampleRate) :
    index_{ 0 },
    sampleRate_{ sampleRate },
    baseSampleRate_{ sampleRate },
    samplesWritten_{ },
    targetLatency_{ sampleRate } // 1 frame
{
}

void DynamicSampleRate::OnFrame(uint64_t samplesWritten, uint64_t samplesPlayed)
{
    samplesWritten_ += samplesWritten;

    int64_t latency = samplesWritten_ - samplesPlayed - targetLatency_;

    // tend towards the difference being 0
    sampleRate_ = baseSampleRate_ - (latency - targetLatency_) / 100;
}

void DynamicSampleRate::Reset()
{
    samplesWritten_ = 0;
    sampleRate_ = baseSampleRate_;
}

uint32_t DynamicSampleRate::SampleRate() const
{
    return sampleRate_;
}

uint32_t DynamicSampleRate::TargetLatency() const
{
    return targetLatency_;
}
