#pragma once

#include <cstdint>
#include "../ChrA12Sensitivity.h"
#include "../MirrorMode.h"

struct MMC3State
{
    uint32_t BankSelect;

    uint32_t PrgMode;
    uint32_t PrgBank0;
    uint32_t PrgBank1;
    uint32_t PrgBank2;

    uint32_t PrgRamProtect0;

    uint32_t ChrMode{};

    bool IrqEnabled;
    uint32_t IrqCounter;
    bool ReloadCounter;
    uint32_t ReloadValue;
};

class Bus;
struct CartCoreState;
struct CartData;

struct MMC3 {
    static void Initialize(CartCoreState& state, CartData& data);

    static void Write0x8xxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void Write0xaxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void Write0xcxxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void Write0xexxx(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value);

    static void A12Rising(Bus& bus, CartCoreState& state, CartData& data);

    static void SetBank(Bus& bus, CartCoreState& state, CartData& data, uint32_t bank);
    static void ClockIrqCounter(Bus& bus, CartCoreState& state);

    static void UpdatePrgMap(CartCoreState& state, CartData& data);
    static void UpdateChrMap(CartCoreState& state, CartData& data);
    static void UpdateChrA12Sensitivity(Bus& bus, CartCoreState& state, ChrA12Sensitivity activeSensitivity);
};