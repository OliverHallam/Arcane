#pragma once

enum class MenuCommand : WORD
{
    Open = 101,
    Close = 102,
    Exit= 103,

    Restart = 201,
    Snapshot = 202,
    Restore = 203,

    Fullscreen = 301
};

class Menu
{
public:
    Menu();

    HMENU Get() const;
    HACCEL AcceleratorTable() const;

    void UpdateLoaded(bool isLoaded);

private:
    HMENU fileMenu_;
    HMENU gameMenu_;
    HMENU videoMenu_;

    HMENU menuBar_;

    HACCEL acceleratorTable_;
};