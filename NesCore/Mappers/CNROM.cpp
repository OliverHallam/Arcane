#include "CNROM.h"

#include "MapperUtils.h"
#include "..\Bus.h"
#include "..\CartCoreState.h"
#include "..\CartData.h"

void CNROM::Initialize(CartCoreState& state, bool busConflicts)
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

    auto& cnromState = state.MapperState.CNROM;
    cnromState.ChrBank = 0;

}

void CNROM::Write(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    state.MapperState.CNROM.ChrBank = value;

    bus.SyncPpu();
    UpdateChrMap8k(state, data, value);
}

void CNROM::WriteWithConflicts(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t value)
{
    auto bank = state.CpuBanks[address >> 13];
    value &= bank[address & 0x1fff];

    Write(bus, state, data, address, value);
}

void CNROM::Write2(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
    Write(bus, state, data, address, firstValue);
    bus.TickCpuWrite();
    Write(bus, state, data, address, secondValue);
}

void CNROM::Write2WithConflicts(Bus& bus, CartCoreState& state, CartData& data, uint16_t address, uint8_t firstValue, uint8_t secondValue)
{
    WriteWithConflicts(bus, state, data, address, firstValue);
    bus.TickCpuWrite();
    WriteWithConflicts(bus, state, data, address, secondValue);
}
