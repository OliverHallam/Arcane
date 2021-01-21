#include "NesSystem.h"
#include "SystemState.h"

NesSystem::NesSystem(uint32_t audioSampleRate)
    : display_{},
    bus_{},
    ppu_{ bus_, display_ },
    cpu_{ bus_ },
    apu_{ bus_, audioSampleRate / 60 },
    controller1_{},
    controller2_{}
{
    bus_.Attach(&ppu_);
    bus_.Attach(&cpu_);
    bus_.Attach(&apu_);
    bus_.AttachPlayer1(&controller1_);
    bus_.AttachPlayer2(&controller2_);
}

Controller& NesSystem::Controller1()
{
    return controller1_;
}

Controller& NesSystem::Controller2()
{
    return controller2_;
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

std::unique_ptr<Cart> NesSystem::RemoveCart()
{
    bus_.DetachCart();
    return std::move(cart_);
}

bool NesSystem::HasCart() const
{
    return bus_.HasCart();
}

void NesSystem::Reset()
{
    // TODO: implement reset fully.
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
    controller1_.CaptureState(&state->Controller1State);
    controller2_.CaptureState(&state->Controller2State);
    cart_->CaptureState(&state->CartState);
}
void NesSystem::RestoreState(const SystemState& state)
{
    bus_.RestoreState(state.BusState);
    cpu_.RestoreState(state.CpuState);
    ppu_.RestoreState(state.PpuState);
    apu_.RestoreState(state.ApuState);
    controller1_.RestoreState(state.Controller1State);
    controller2_.RestoreState(state.Controller2State);
    cart_->RestoreState(state.CartState);
}
