#pragma once

#include <cstdint>

class DynamicSampleRate
{
public:
    DynamicSampleRate(uint32_t sampleRate);

    void OnFrame(uint64_t samplesWritten, uint64_t samplesPlayed);

    uint32_t SampleRate() const;

private:
    uint64_t samplesWritten_;

    std::array<uint64_t, 30> samplesPlayed_;

    uint32_t index_;

    uint32_t sampleRate_;
    uint32_t baseSampleRate_;
};