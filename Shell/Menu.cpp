#include "pch.h"

#include "Menu.h"


Menu::Menu()
{
    menuBar_ = CreateMenu();

    fileMenu_ = CreateMenu();
    AppendMenu(fileMenu_, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void *>(MenuCommand::Open)), L"&Open ROM...\tCtrl+O");

    AppendMenu(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(fileMenu_), L"&File");

    gameMenu_ = CreateMenu();
    AppendMenu(gameMenu_, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Snapshot)), L"Create &Snapshot\tF6");
    AppendMenu(gameMenu_, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void*>(MenuCommand::Restore)), L"&Restore Snapshot\tF9");
    
    AppendMenu(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(gameMenu_), L"&Game");


    ACCEL accelerators[] =
    {
        { FCONTROL | FVIRTKEY, 'O', static_cast<WORD>(MenuCommand::Open) },

        { FVIRTKEY, VK_F6, static_cast<WORD>(MenuCommand::Snapshot) },
        { FVIRTKEY, VK_F9, static_cast<WORD>(MenuCommand::Restore) }, 
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
