#pragma once

enum class MenuCommand : WORD
{
    Open = 101,

    Snapshot = 201,
    Restore = 202
};

class Menu
{
public:
    Menu();

    HMENU Get() const;
    HACCEL AcceleratorTable() const;

private:
    HMENU fileMenu_;
    HMENU gameMenu_;
    HMENU menuBar_;

    HACCEL acceleratorTable_;
};