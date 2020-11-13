#include "pch.h"

#include "Menu.h"


Menu::Menu()
{
    menuBar_ = CreateMenu();

    fileMenu_ = CreateMenu();
    AppendMenu(fileMenu_, MF_ENABLED, reinterpret_cast<UINT_PTR>(reinterpret_cast<void *>(MenuCommand::Open)), L"Open ROM...\tCtrl+O");

    AppendMenu(menuBar_, MF_POPUP, reinterpret_cast<UINT_PTR>(fileMenu_), L"File");


    ACCEL accelerators[] =
    {
        { FCONTROL | FVIRTKEY, 'O', static_cast<WORD>(MenuCommand::Open) }
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
