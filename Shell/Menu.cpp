#include "pch.h"

#include "Menu.h"


Menu::Menu()
{
    menuBar_ = CreateMenu();

    auto fileMenu = CreateMenu();
    AppendMenu(fileMenu, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void *>(MenuCommand::Open)), L"&Open ROM...\tCtrl+O");
    AppendMenu(fileMenu, MF_DISABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Close)), L"Close");
    AppendMenu(fileMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(fileMenu, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Exit)), L"Exit\tAlt+F4");

    AppendMenu(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(fileMenu), L"&File");

    auto gameMenu = CreateMenu();
    AppendMenu(gameMenu, MF_DISABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Restart)), L"Restart");
    AppendMenu(gameMenu, MF_SEPARATOR, NULL, NULL);
    AppendMenu(gameMenu, MF_DISABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Snapshot)), L"Create &Snapshot\tF6");
    AppendMenu(gameMenu, MF_DISABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Restore)), L"&Restore Snapshot\tF9");

    AppendMenu(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(gameMenu), L"&Game");

    auto inputMenu = CreateMenu();
    auto player1Menu = CreateControllerMenu(static_cast<WORD>(MenuCommand::InputPlayer1Keyboard));
    auto player2Menu = CreateControllerMenu(static_cast<WORD>(MenuCommand::InputPlayer2Keyboard));
    AppendMenu(inputMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(player1Menu), L"Player 1");
    AppendMenu(inputMenu, MF_POPUP, reinterpret_cast<UINT_PTR>(player2Menu), L"Player 2");

    AppendMenu(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(inputMenu), L"&Input");

    auto videoMenu = CreateMenu();
    AppendMenu(videoMenu, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Fullscreen)), L"Toggle &Fullscreen\tF11");
    AppendMenu(videoMenu, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Overscan)), L"Crop &Overscan");
    AppendMenu(videoMenu, MF_ENABLED | MF_CHECKED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::IntegerScaling)), L"Pixel Perfect Scaling");
    AppendMenu(videoMenu, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Scanlines)), L"Scanline Filter");

    AppendMenu(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(videoMenu), L"&Video");

    ACCEL accelerators[] =
    {
        { FCONTROL | FVIRTKEY, 'O', static_cast<WORD>(MenuCommand::Open) },

        { FVIRTKEY, VK_F6, static_cast<WORD>(MenuCommand::Snapshot) },
        { FVIRTKEY, VK_F9, static_cast<WORD>(MenuCommand::Restore) }, 

        { FVIRTKEY, VK_F11, static_cast<WORD>(MenuCommand::Fullscreen) }
    };

    acceleratorTable_ = CreateAcceleratorTable(accelerators, _countof(accelerators));
}

HMENU Menu::CreateControllerMenu(UINT_PTR baseId)
{
    auto controllerMenu = CreatePopupMenu();
    AppendMenu(controllerMenu, MF_ENABLED, baseId, L"Keyboard");
    AppendMenu(controllerMenu, MF_ENABLED, baseId + 1, L"Controller 1");
    AppendMenu(controllerMenu, MF_ENABLED, baseId + 2, L"Controller 2");
    AppendMenu(controllerMenu, MF_ENABLED, baseId + 3, L"Controller 3");
    AppendMenu(controllerMenu, MF_ENABLED, baseId + 4, L"Controller 4");
    return controllerMenu;
}

HMENU Menu::Get() const
{
    return menuBar_;
}

HACCEL Menu::AcceleratorTable() const
{
    return acceleratorTable_;
}

void Menu::SetLoaded(bool isLoaded)
{
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::Close), isLoaded ? MF_ENABLED : MF_DISABLED);
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::Restart), isLoaded ? MF_ENABLED : MF_DISABLED);
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::Snapshot), isLoaded ? MF_ENABLED : MF_DISABLED);
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::Restore), isLoaded ? MF_ENABLED : MF_DISABLED);
}

void Menu::SetOverscan(bool overscan)
{
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::Overscan), overscan ? MF_CHECKED : MF_UNCHECKED);
}

void Menu::SetIntegerScaling(bool integerScaling)
{
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::IntegerScaling), integerScaling ? MF_CHECKED : MF_UNCHECKED);
}

void Menu::SetScanlines(bool scanlines)
{
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::Scanlines), scanlines ? MF_CHECKED : MF_UNCHECKED);
}

void Menu::SetControllerConnected(bool controller0, bool controller1, bool controller2, bool controller3)
{
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer1Controller0), controller0 ? MF_ENABLED : MF_DISABLED);
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer1Controller1), controller1 ? MF_ENABLED : MF_DISABLED);
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer1Controller2), controller2 ? MF_ENABLED : MF_DISABLED);
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer1Controller3), controller3 ? MF_ENABLED : MF_DISABLED);
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer2Controller0), controller0 ? MF_ENABLED : MF_DISABLED);
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer2Controller1), controller1 ? MF_ENABLED : MF_DISABLED);
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer2Controller2), controller2 ? MF_ENABLED : MF_DISABLED);
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer2Controller3), controller3 ? MF_ENABLED : MF_DISABLED);
}

void Menu::SetPlayer1Device(InputDevice device)
{
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer1Keyboard), device == InputDevice::Keyboard ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer1Controller0), device == InputDevice::Controller0 ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer1Controller1), device == InputDevice::Controller1 ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer1Controller2), device == InputDevice::Controller2 ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer1Controller3), device == InputDevice::Controller3 ? MF_CHECKED : MF_UNCHECKED);
}

void Menu::SetPlayer2Device(InputDevice device)
{
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer2Keyboard), device == InputDevice::Keyboard ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer2Controller0), device == InputDevice::Controller0 ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer2Controller1), device == InputDevice::Controller1 ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer2Controller2), device == InputDevice::Controller2 ? MF_CHECKED : MF_UNCHECKED);
    CheckMenuItem(menuBar_, static_cast<UINT>(MenuCommand::InputPlayer2Controller3), device == InputDevice::Controller3 ? MF_CHECKED : MF_UNCHECKED);
}
