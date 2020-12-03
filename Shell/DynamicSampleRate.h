#pragma once

#include <cstdint>

class DynamicSampleRate
{
public:
    DynamicSampleRate(uint32_t sampleRate);

    void OnFrame(uint64_t samplesWritten, uint64_t samplesPlayed);

    void Reset();

    uint32_t SampleRate() const;

    uint32_t TargetLatency() const;

private:
    uint64_t samplesWritten_;

    uint32_t index_;

    int32_t latency_;
    int32_t targetLatency_;

    uint32_t baseSampleRate_;
};