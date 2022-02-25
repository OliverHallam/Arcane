#pragma once

#include "MMC3.h"

#include "..\MapperType.h"

struct Rambo1State : public MMC3State
{
    uint32_t IrqMode;

    uint32_t LastA12Cycle;
    uint32_t PrescalerResetCycle;
    bool BumpIrqCounter;
};

struct Rambo1
{
    static void Initialize(CartCoreState& state, CartData& data);

    static void Write0x8xxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void Write0xaxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void Write0xcxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void Write0xexxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);

    static void A12Rising(Bus& bus, CartCoreState& state, CartData& data);
    static void ClockCpuIrqCounter(Bus& bus, CartCoreState& state);

    static void ClockIrqCounter(Bus& bus, CartCoreState& state);
};