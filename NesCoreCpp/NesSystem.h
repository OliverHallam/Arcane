#pragma once

#include <memory>

#include "Bus.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Apu.h"
#include "Display.h"
#include "Controller.h"

class NesSystem
{
public:
    NesSystem(uint32_t audioSampleRate);

    Controller& Controller();
    const Display& Display() const;
    const Apu& Apu() const;

    void InsertCart(std::unique_ptr<Cart> cart);

    void Reset();

    void RunFrame();

private:
    Bus bus_;
    Cpu cpu_;
    Ppu ppu_;
    ::Apu apu_;
    ::Display display_;
    ::Controller controller_;
};