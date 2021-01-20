#pragma once

#include "InputDevice.h"

enum class MenuCommand : WORD
{
    Open = 101,
    Close = 102,
    Exit = 103,

    Restart = 201,
    Snapshot = 202,
    Restore = 203,
    RewindEnabled = 204,

    InputPlayer1Keyboard = 301,
    InputPlayer1Controller0 = 302,
    InputPlayer1Controller1 = 303,
    InputPlayer1Controller2 = 304,
    InputPlayer1Controller3 = 305,
    InputPlayer2Keyboard = 311,
    InputPlayer2Controller0 = 312,
    InputPlayer2Controller1 = 313,
    InputPlayer2Controller2 = 314,
    InputPlayer2Controller3 = 315,

    Fullscreen = 401,
    Overscan = 402,
    IntegerScaling = 403,
    Scanlines = 404
};

class Menu
{
public:
    Menu();

    HMENU Get() const;
    HACCEL AcceleratorTable() const;

    void SetRewindEnabled(bool enabled);

    void SetLoaded(bool isLoaded);
    void SetOverscan(bool overscan);
    void SetIntegerScaling(bool integerScaling);
    void SetScanlines(bool scanlines);

    void SetControllerConnected(bool controller0, bool controller1, bool controller2, bool controller3);
    void SetPlayer1Device(InputDevice device);
    void SetPlayer2Device(InputDevice device);

private:
    HMENU CreateControllerMenu(UINT_PTR baseId);

    HMENU menuBar_;

    HACCEL acceleratorTable_;
};