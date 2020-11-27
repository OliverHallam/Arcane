#include "pch.h"

#include "DynamicSampleRate.h"

DynamicSampleRate::DynamicSampleRate(uint32_t sampleRate) :
    index_{ 0 },
    sampleRate_{ sampleRate },
    baseSampleRate_{ sampleRate }, // 1 frame
    samplesWritten_{ } // buy ourselves a little wriggle room
{
}

void DynamicSampleRate::OnFrame(uint64_t samplesWritten, uint64_t samplesPlayed)
{
    samplesWritten_ += samplesWritten;

    int64_t difference = samplesWritten_ - samplesPlayed;

    // tend towards the difference being 0
    sampleRate_ = baseSampleRate_ - difference / 100;
}

uint32_t DynamicSampleRate::SampleRate() const
{
    return sampleRate_;
}
