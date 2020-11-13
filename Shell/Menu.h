#pragma once

enum class MenuCommand : WORD
{
    Open = 101
};

class Menu
{
public:
    Menu();

    HMENU Get() const;
    HACCEL AcceleratorTable() const;

private:
    HMENU fileMenu_;
    HMENU menuBar_;

    HACCEL acceleratorTable_;
};