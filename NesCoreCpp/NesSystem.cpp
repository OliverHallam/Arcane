#include "NesSystem.h"
#include "SystemState.h"

NesSystem::NesSystem(uint32_t audioSampleRate)
    : display_{},
    bus_{},
    ppu_{ bus_, display_ },
    cpu_{ bus_ },
    apu_{ bus_, audioSampleRate / 60 },
    controller_{}
{
    bus_.Attach(&ppu_);
    bus_.Attach(&cpu_);
    bus_.Attach(&apu_);
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

Apu& NesSystem::Apu()
{
    return apu_;
}

void NesSystem::InsertCart(std::unique_ptr<Cart> cart)
{
    cart_ = std::move(cart);
    bus_.Attach(cart_.get());
}

void NesSystem::RemoveCart()
{
    bus_.DetachCart();
}

bool NesSystem::HasCart() const
{
    return bus_.HasCart();
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

void NesSystem::CaptureState(SystemState* state) const
{
    bus_.CaptureState(&state->BusState);
    cpu_.CaptureState(&state->CpuState);
    ppu_.CaptureState(&state->PpuState);
    apu_.CaptureState(&state->ApuState);
    controller_.CaptureState(&state->ControllerState);
    cart_->CaptureState(&state->CartState);
}
void NesSystem::RestoreState(const SystemState& state)
{
    bus_.RestoreState(state.BusState);
    cpu_.RestoreState(state.CpuState);
    ppu_.RestoreState(state.PpuState);
    apu_.RestoreState(state.ApuState);
    controller_.RestoreState(state.ControllerState);
    cart_->RestoreState(state.CartState);
}
