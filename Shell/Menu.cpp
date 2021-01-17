#include "pch.h"

#include "Menu.h"


Menu::Menu()
{
    menuBar_ = CreateMenu();

    fileMenu_ = CreateMenu();
    AppendMenu(fileMenu_, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void *>(MenuCommand::Open)), L"&Open ROM...\tCtrl+O");
    AppendMenu(fileMenu_, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Close)), L"Close");

    AppendMenu(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(fileMenu_), L"&File");

    gameMenu_ = CreateMenu();
    AppendMenu(gameMenu_, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Snapshot)), L"Create &Snapshot\tF6");
    AppendMenu(gameMenu_, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Restore)), L"&Restore Snapshot\tF9");

    AppendMenu(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(gameMenu_), L"&Game");

    videoMenu_ = CreateMenu();
    AppendMenu(videoMenu_, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Fullscreen)), L"Toggle &Fullscreen\tF11");

    AppendMenu(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(videoMenu_), L"&Video");

    ACCEL accelerators[] =
    {
        { FCONTROL | FVIRTKEY, 'O', static_cast<WORD>(MenuCommand::Open) },

        { FVIRTKEY, VK_F6, static_cast<WORD>(MenuCommand::Snapshot) },
        { FVIRTKEY, VK_F9, static_cast<WORD>(MenuCommand::Restore) }, 

        { FVIRTKEY, VK_F11, static_cast<WORD>(MenuCommand::Fullscreen) }
    };

    acceleratorTable_ = CreateAcceleratorTable(accelerators, _countof(accelerators));
}

HMENU Menu::Get() const
{
    return menuBar_;
}

HACCEL Menu::AcceleratorTable() const
{
    return acceleratorTable_;
}

void Menu::UpdateLoaded(bool isLoaded)
{
    EnableMenuItem(menuBar_, static_cast<UINT>(MenuCommand::Close), isLoaded ? MF_ENABLED : MF_DISABLED);
}
