#include "UxROM.h"

#include "..\Bus.h"
#include "..\CartCoreState.h"
#include "..\CartData.h"

void UxROM::Initialize(CartCoreState& state, const CartData& data, bool busConflicts)
{
    if (busConflicts)
    {
        state.WriteMap[4] = WriteWithConflicts;
        state.WriteMap[5] = WriteWithConflicts;
        state.WriteMap[6] = WriteWithConflicts;
        state.WriteMap[7] = WriteWithConflicts;

        state.Write2Map[4] = Write2WithConflicts;
        state.Write2Map[5] = Write2WithConflicts;
        state.Write2Map[6] = Write2WithConflicts;
        state.Write2Map[7] = Write2WithConflicts;
    }
    else
    {
        state.WriteMap[4] = Write;
        state.WriteMap[5] = Write;
        state.WriteMap[6] = Write;
        state.WriteMap[7] = Write;

        state.Write2Map[4] = Write2;
        state.Write2Map[5] = Write2;
        state.Write2Map[6] = Write2;
        state.Write2Map[7] = Write2;
    }

    auto& uxromState = state.MapperState.UxROM;
    // TODO!
    uxromState.PrgBank = data.PrgMask >> 14;
}

void UxROM::Write(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    state.MapperState.UxROM.PrgBank = value;

    auto bankAddress = (value << 14) & data.PrgMask;
    auto base = &data.PrgData[bankAddress];
    state.CpuBanks[4] = base;
    state.CpuBanks[5] = base + 0x2000;
}

void UxROM::WriteWithConflicts(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto bank = state.CpuBanks[address >> 13];
    value &= bank[address & 0x1fff];

    Write(bus, state, data, address, value);
}

void UxROM::Write2(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
    bus.TickCpuWrite();
    Write(bus, state, data, address, secondValue);
}

void UxROM::Write2WithConflicts(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
    bus.TickCpuWrite();
    WriteWithConflicts(bus, state, data, address, secondValue);
}
