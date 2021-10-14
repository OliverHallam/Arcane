#pragma once

#include <cstdint>
#include "../MirrorMode.h"

struct MMC1State
{
    uint32_t MapperShiftCount;
    uint32_t MapperShift;

    uint32_t BankSelect;
    uint32_t CommandNumber;

    MirrorMode MirrorMode{ MirrorMode::Horizontal };
    
    uint32_t ChrMode;
    uint32_t ChrBank0;
    uint32_t ChrBank1;

    uint32_t PrgMode;
    uint32_t PrgPlane0;
    uint32_t PrgPlane1;
    uint32_t PrgBank;
    uint32_t PrgRamEnabled{};
    uint32_t PrgRamBank0{};
    uint32_t PrgRamBank1{};
};

class Bus;
struct CartCoreState;
struct CartData;

struct MMC1 {
    static void Initialize(CartCoreState& state);

    static void Write(CartCoreState& state, CartData& data, uint16_t address, uint8_t value);
    static void Write2(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue);

    static void A12Rising(CartCoreState& state, CartData& data);
    static void A12Falling(CartCoreState& state, CartData& data);

private:
    static void WriteRegister(CartCoreState& state, CartData& data,  uint16_t address, uint8_t value);

    static void UpdateChrMap(CartCoreState& state, CartData& data);
    static void UpdatePrgMap(CartCoreState& state, CartData& data);
};