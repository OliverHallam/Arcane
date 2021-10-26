#pragma once

#include <cstdint>
#include "MMC3.h"

struct MMC6State : public MMC3State
{

    uint32_t PrgRamProtect1;
};

class Bus;
struct CartCoreState;
struct CartData;

struct MMC6 {
    static void Initialize(CartCoreState& state, CartData& data);

    static void Write0x8xxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void Write0xaxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);

    static uint8_t Read0x6xxx(CartCoreState& state, CartData& data, uint16_t address);

    static void UpdatePrgMap(CartCoreState& state, CartData& data);
    static void UpdateChrMap(CartCoreState& state, CartData& data);
};