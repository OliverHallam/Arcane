#include "NesSystem.h"

NesSystem::NesSystem()
    : display_{},
    bus_{},
    ppu_{ bus_, display_ },
    cpu_{ bus_ },
    controller_{}
{
    bus_.Attach(&ppu_);
    bus_.Attach(&cpu_);
    bus_.Attach(&controller_);
}

Controller& NesSystem::Controller()
{
    return controller_;
}

const Display& NesSystem::Display() const
{
    return display_;
}

void NesSystem::InsertCart(std::unique_ptr<Cart> cart)
{
    bus_.Attach(std::move(cart));
}

void NesSystem::Reset()
{
    cpu_.Reset();
}

void NesSystem::RunFrame()
{
    int currentFrame = ppu_.FrameCount();

    do
    {
        cpu_.RunInstruction();
    } while (ppu_.FrameCount() == currentFrame);
}
