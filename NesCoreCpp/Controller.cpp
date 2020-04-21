#include "Controller.h"

void Controller::Up(bool pressed)
{
    up_ = pressed;
}

void Controller::Down(bool pressed)
{
    down_ = pressed;
}

void Controller::Left(bool pressed)
{
    left_ = pressed;
}

void Controller::Right(bool pressed)
{
    right_ = pressed;
}

void Controller::A(bool pressed)
{
    a_ = pressed;
}

void Controller::B(bool pressed)
{
    b_ = pressed;
}

void Controller::Select(bool pressed)
{
    select_ = pressed;
}

void Controller::Start(bool pressed)
{
    start_ = pressed;
}

uint8_t Controller::Read()
{
    if (strobe_)
    {
        CaptureState();
    }

    auto status = state_ >> 7;
    state_ <<= 1;
    state_ |= 1;

    return noise_ | status;
}

void Controller::Write(uint8_t data)
{
    if (!(data & 0x01) && strobe_)
    {
        CaptureState();
    }

    noise_ = data & 0xf4;
    strobe_ = data & 0x01;
}

void Controller::CaptureState()
{
    state_ = 0;
    if (a_)
        state_ |= 0x80;
    if (b_)
        state_ |= 0x40;
    if (select_)
        state_ |= 0x20;
    if (start_)
        state_ |= 0x10;

    // we prevent inputs that are supposed to be impossible from a NES controller here - this can cause horrible
    // crashes in some games, e.g. Burger Time
    if (up_ && !down_)
        state_ |= 0x08;
    if (down_ && !up_)
        state_ |= 0x04;
    if (left_ && !right_)
        state_ |= 0x02;
    if (right_ && !left_)
        state_ |= 0x01;
}
