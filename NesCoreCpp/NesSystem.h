#pragma once

#include <memory>

#include "Bus.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Apu.h"
#include "Cart.h"
#include "Display.h"
#include "Controller.h"

struct SystemState;

class NesSystem
{
public:
    NesSystem(uint32_t audioSampleRate);

    Controller& Controller1();
    Controller& Controller2();
    const Display& Display() const;
    Apu& Apu();

    void InsertCart(std::unique_ptr<Cart> cart);
    std::unique_ptr<Cart> RemoveCart();
    bool HasCart() const;

    void Reset();

    void RunFrame();

    void CaptureState(SystemState* state) const;
    void RestoreState(const SystemState& state);

private:
    Bus bus_;
    Cpu cpu_;
    Ppu ppu_;
    ::Apu apu_;
    ::Display display_;
    ::Controller controller1_;
    ::Controller controller2_;
    std::unique_ptr<Cart> cart_;
};