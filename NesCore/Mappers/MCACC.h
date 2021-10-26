#pragma once

#include "MMC3.h"

struct MCACCState : public MMC3State
{
    uint32_t ChrA12PulseCounter;
};

struct MCACC
{
    static void Initialize(CartCoreState& state, CartData& data);

    static void Write0xcxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void Write0xexxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);

    static void A12Falling(Bus& bus, CartCoreState& state);
};