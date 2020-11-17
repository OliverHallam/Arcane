#include "Controller.h"

void Controller::Up(bool pressed)
{
    state_.Up = pressed;
}

void Controller::Down(bool pressed)
{
    state_.Down = pressed;
}

void Controller::Left(bool pressed)
{
    state_.Left = pressed;
}

void Controller::Right(bool pressed)
{
    state_.Right = pressed;
}

void Controller::A(bool pressed)
{
    state_.A = pressed;
}

void Controller::B(bool pressed)
{
    state_.B = pressed;
}

void Controller::Select(bool pressed)
{
    state_.Select = pressed;
}

void Controller::Start(bool pressed)
{
    state_.Start = pressed;
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
    state_.State = 0;
    if (state_.A)
        state_.State |= 0x80;
    if (state_.B)
        state_.State |= 0x40;
    if (state_.Select)
        state_.State |= 0x20;
    if (state_.Start)
        state_.State |= 0x10;

    // we prevent inputs that are supposed to be impossible from a NES controller here - this can cause horrible
    // crashes in some games, e.g. Burger Time
    if (state_.Up && !state_.Down)
        state_.State |= 0x08;
    if (state_.Down && !state_.Up)
        state_.State |= 0x04;
    if (state_.Left && !state_.Right)
        state_.State |= 0x02;
    if (state_.Right && !state_.Left)
        state_.State |= 0x01;
}
