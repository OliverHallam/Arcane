#include "CpuState.h"

uint8_t CpuState::P()
{
    return
        (N ? 0x80 : 0) |
        (V ? 0x40 : 0) |
        (D ? 0x08 : 0) |
        (I ? 0x04 : 0) |
        (Z ? 0x02 : 0) |
        (C ? 0x01 : 0);
}

void CpuState::P(uint8_t value)
{
    N = (value & 0x80) != 0;
    V = (value & 0x40) != 0;
    D = (value & 0x08) != 0;
    I = (value & 0x04) != 0;
    Z = (value & 0x02) != 0;
    C = (value & 0x01) != 0;
}
