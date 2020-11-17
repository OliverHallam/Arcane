#pragma once

#include "ControllerState.h"

#include <cstdint>

class Controller
{
public:
    void Up(bool pressed);
    void Down(bool pressed);
    void Left(bool pressed);
    void Right(bool pressed);

    void A(bool pressed);
    void B(bool pressed);

    void Select(bool pressed);
    void Start(bool pressed);

    uint8_t Read();
    void Write(uint8_t data);

    void CaptureState(ControllerState* state) const;
    void RestoreState(const ControllerState& state);

private:
    void CaptureState();

    ControllerState state_;
};