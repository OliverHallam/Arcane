#pragma once

#include <cstdint>

class Bus;

class ApuDmc
{
public:
    ApuDmc(Bus& bus);

    void Enable(bool enabled);
    bool IsEnabled() const;

    void Write(uint16_t address, uint8_t value);

    void Run(uint32_t cycles);

    void SetBuffer(uint8_t value);

    uint8_t Sample() const;

    uint32_t CyclesUntilNextDma();

private:
    void Clock();

    static uint32_t GetLinearRate(uint8_t rate);

    bool enabled_;
    bool irqEnabled_;
    bool loop_;
    uint32_t rate_;
    uint8_t level_;
    uint16_t sampleAddress_;
    uint16_t sampleLength_;

    int32_t timer_;

    uint8_t outBuffer_;
    bool outBufferHasData_;
    uint8_t sampleShift_;

    uint8_t sampleBuffer_;
    bool sampleBufferHasData_;

    uint16_t currentAddress_;
    uint16_t sampleBytesRemaining_;

    Bus& bus_;
};