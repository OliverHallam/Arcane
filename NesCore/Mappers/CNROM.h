#pragma once

#include <cstdint>

class Bus;
struct CartCoreState;
struct CartData;

struct CNROMState
{
    uint32_t ChrBank;
};

struct CNROM
{
    static void Initialize(CartCoreState& state, bool busConflicts);

    static void Write(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void WriteWithConflicts(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void Write2(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue);
    static void Write2WithConflicts(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue);
};