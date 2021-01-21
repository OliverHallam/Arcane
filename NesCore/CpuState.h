#pragma once

#include <cstdint>

struct CpuState
{
    // Registers
    uint8_t A{};
    uint8_t X{};
    uint8_t Y{};
    uint16_t PC{};
    uint8_t S{};

    // the flags register as seperate bytes
    bool C{}, Z{}, I{}, D{}, B{}, V{}, N{};
    uint8_t P();
    void P(uint8_t value);

    uint16_t InterruptVector{};
    bool Irq{};
    bool SkipInterrupt{};
};