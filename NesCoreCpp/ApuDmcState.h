#pragma once

#include <cstdint>

struct ApuDmcState
{
    bool irqEnabled_{};
    bool loop_{};
    uint32_t rate_{ 428 };
    uint8_t level_{};
    uint16_t sampleAddress_{ 0xC000 };
    uint16_t sampleLength_{ 1 };

    int32_t timer_{ 428 };

    uint8_t outBuffer_{};
    bool outBufferHasData_{};
    uint8_t sampleShift_{};

    uint8_t sampleBuffer_{};
    bool sampleBufferHasData_{};

    uint16_t currentAddress_{ 0xC000 };
    uint16_t sampleBytesRemaining_{};
};