#pragma once

#include "InputDevice.h"

#include <array>
#include <cstdint>

class Input
{
public:
    Input();

    void CheckForNewControllers();
    void UpdateControllerState();

    bool IsConnected(InputDevice device);
    uint32_t GetState(InputDevice device);

    bool ProcessKey(uint32_t key, bool pressed);



private:
    int32_t GetButton(uint32_t key);

    uint32_t keyboardState_;

    std::array<bool, 4> connected_;
    std::array<uint32_t, 4> controllerState_;
};