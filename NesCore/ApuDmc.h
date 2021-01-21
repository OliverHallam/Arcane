#pragma once

#include "ApuDmcState.h"

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

    void CaptureState(ApuDmcState* state) const;
    void RestoreState(const ApuDmcState& state);

private:
    void Clock();
    void RequestByte();

    static uint32_t GetLinearRate(uint8_t rate);

    Apu& apu_;

    ApuDmcState state_;
};