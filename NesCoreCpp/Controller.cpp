#include "Controller.h"


void Controller::ButtonDown(Buttons button)
{
    state_.Buttons |= button;
}

void Controller::ButtonUp(Buttons button)
{
    state_.Buttons &= ~button;
}

void Controller::SetButtonState(int32_t buttons)
{
    state_.Buttons = buttons;
}

uint8_t Controller::Read()
{
    if (state_.Strobe)
    {
        CaptureState();
    }

    auto status = state_.State >> 7;
    state_.State <<= 1;
    state_.State |= 1;

    return state_.Noise | status;
}

void Controller::Write(uint8_t data)
{
    if (!(data & 0x01) && state_.Strobe)
    {
        CaptureState();
    }

    state_.Noise = data & 0xf4;
    state_.Strobe = data & 0x01;
}

void Controller::CaptureState(ControllerState* state) const
{
    *state = state_;
}

void Controller::RestoreState(const ControllerState& state)
{
    state_ = state;
}

void Controller::CaptureState()
{
    uint32_t buttons = state_.Buttons;

    // Prevent inputs that are supposed to be impossible from a NES controller here - this can cause horrible
    // crashes in some games, e.g. Burger Time
    auto upAndDown = (Buttons::Up | Buttons::Down);
    if ((buttons & upAndDown) == upAndDown)
        buttons &= ~upAndDown;

    auto leftAndRight = (Buttons::Left | Buttons::Right);
    if ((buttons & leftAndRight) == leftAndRight)
        buttons &= ~leftAndRight;

    state_.State = static_cast<uint8_t>(buttons);
}
