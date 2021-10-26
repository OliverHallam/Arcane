#pragma once

#include "MMC3.h"

struct Rambo1State : public MMC3State
{
    bool IrqEnabled{};
    uint32_t IrqCounter{};
    bool ReloadCounter{};
    uint8_t ReloadValue{};
};

struct Rambo1
{
    static void Initialize(CartCoreState& state, CartData& data);

    static void ClockIrqCounter(Bus& bus, CartCoreState& state);
};