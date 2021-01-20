#pragma once

#include <cstdint>

struct ControllerState
{
    uint32_t Buttons;

    bool Strobe;
    uint8_t Noise;
    uint8_t State;
};