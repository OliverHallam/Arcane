#pragma once

#include "Buttons.h"
#include "ControllerState.h"

#include <cstdint>

class Controller
{
public:
    void ButtonDown(Buttons button);
    void ButtonUp(Buttons button);

    void SetButtonState(int32_t buttons);

    uint8_t Read();
    void Write(uint8_t data);

    void CaptureState(ControllerState* state) const;
    void RestoreState(const ControllerState& state);

private:
    void CaptureState();

    ControllerState state_;
};