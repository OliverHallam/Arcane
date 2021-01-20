#include "pch.h"

#include "Input.h"

#include "../NesCoreCpp/Buttons.h"

#include "ExtendedButtons.h"

#include <winerror.h>
#include <Xinput.h>

Input::Input() :
    connected_{},
    keyboardState_{},
    controllerState_{}
{
}

void Input::CheckForNewControllers()
{
    for (auto i = 0; i < 4; i++)
    {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));

        auto result = XInputGetState(i, &state);
        auto connected = result == ERROR_SUCCESS;
        connected_[i] = connected;
        if (!connected)
            controllerState_[i] = 0;
    }
}

void Input::UpdateControllerState()
{
    for (auto i = 0; i < 4; i++)
    {
        if (!connected_[i])
            continue;

        XINPUT_STATE state;
        auto result = XInputGetState(i, &state);

        // we should handle this though the WM_DEVICECHANGED, but just in case...
        auto connected = result == ERROR_SUCCESS;
        if (!connected)
        {
            connected_[i] = connected;
            controllerState_[i] = 0;
        }

        uint32_t buttons = 0;
        auto gamepadButtons = state.Gamepad.wButtons;

        if (gamepadButtons & XINPUT_GAMEPAD_DPAD_UP)
            buttons |= BUTTON_UP;
        if (gamepadButtons & XINPUT_GAMEPAD_DPAD_DOWN)
            buttons |= BUTTON_DOWN;
        if (gamepadButtons & XINPUT_GAMEPAD_DPAD_LEFT)
            buttons |= BUTTON_LEFT;
        if (gamepadButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
            buttons |= BUTTON_RIGHT;

        // XBOX buttons are the wrong way around compared to Nintendo.
        if ((gamepadButtons & XINPUT_GAMEPAD_A) || (gamepadButtons & XINPUT_GAMEPAD_Y))
            buttons |= BUTTON_A;
        if ((gamepadButtons & XINPUT_GAMEPAD_B) || (gamepadButtons & XINPUT_GAMEPAD_X))
            buttons |= BUTTON_B;
        if ((gamepadButtons & XINPUT_GAMEPAD_START) != 0)
            buttons |= BUTTON_START;
        if ((gamepadButtons & XINPUT_GAMEPAD_BACK) != 0)
            buttons |= BUTTON_SELECT;

        if (state.Gamepad.bLeftTrigger >= 128)
            buttons |= BUTTON_REWIND;

        if (state.Gamepad.sThumbLX >= 16384)
            buttons |= BUTTON_RIGHT;
        if (state.Gamepad.sThumbLX <= -16384)
            buttons |= BUTTON_LEFT;
        if (state.Gamepad.sThumbLY >= 16384)
            buttons |= BUTTON_UP;
        if (state.Gamepad.sThumbLY <= -16384)
            buttons |= BUTTON_DOWN;

        controllerState_[i] = buttons;
    }
}

bool Input::IsConnected(InputDevice device)
{
    switch (device)
    {
    case InputDevice::Keyboard:
        return true;

    case InputDevice::Controller0:
    case InputDevice::Controller1:
    case InputDevice::Controller2:
    case InputDevice::Controller3:
        return connected_[static_cast<int32_t>(device) - static_cast<int32_t>(InputDevice::Controller0)];

    default:
        return false;
    }
}

uint32_t Input::GetState(InputDevice device)
{
    switch (device)
    {
    case InputDevice::Keyboard:
        return keyboardState_;

    case InputDevice::Controller0:
    case InputDevice::Controller1:
    case InputDevice::Controller2:
    case InputDevice::Controller3:
        return controllerState_[static_cast<int32_t>(device) - static_cast<int32_t>(InputDevice::Controller0)];

    default:
        return 0;
    }
}

bool Input::ProcessKey(uint32_t key, bool pressed)
{
    auto button = GetButton(key);
    if (button == 0)
        return false;

    if (pressed)
        keyboardState_ |= button;
    else
        keyboardState_ &= ~button;

    return true;
}

int32_t Input::GetButton(uint32_t key)
{
    switch (key)
    {
    case VK_UP:
        return BUTTON_UP;

    case VK_DOWN:
        return BUTTON_DOWN;

    case VK_LEFT:
        return BUTTON_LEFT;

    case VK_RIGHT:
        return BUTTON_RIGHT;

    case 'Z':
        return BUTTON_B;

    case 'X':
        return BUTTON_A;

    case VK_RETURN:
        return BUTTON_START;

    case VK_SHIFT:
        return BUTTON_SELECT;

    case VK_BACK:
        return BUTTON_REWIND;

    default:
        return 0;
    }
}
