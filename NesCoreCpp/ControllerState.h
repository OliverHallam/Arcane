#pragma once

#include <cstdint>

struct ControllerState
{
    bool Up, Down, Left, Right;
    bool A, B;
    bool Select, Start;

    bool Strobe;
    uint8_t Noise;
    uint8_t State;
};