#pragma once

#include <memory>

#include "Bus.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Display.h"
#include "Controller.h"

class NesSystem
{
public:
    NesSystem();

    Controller& Controller();
    const Display& Display() const;

    void InsertCart(std::unique_ptr<Cart> cart);

    void Reset();

    void RunFrame();

private:
    Bus bus_;
    Cpu cpu_;
    Ppu ppu_;
    ::Display display_;
    ::Controller controller_;
};