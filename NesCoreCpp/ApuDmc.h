#pragma once

#include <cstdint>

class Apu;

class ApuDmc
{
public:
    ApuDmc(Apu& bus);

    void Enable(bool enabled);
    bool IsEnabled() const;

    void Write(uint16_t address, uint8_t value);

    void Run(uint32_t cycles);

    void SetBuffer(uint8_t value);

    uint8_t Sample() const;

private:
    void Clock();
    void RequestByte();

    static uint32_t GetLinearRate(uint8_t rate);

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

    Apu& apu_;
};