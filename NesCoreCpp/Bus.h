#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "Cart.h"
#include "Controller.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Apu.h"

class Bus
{
public:
    Bus();

    void Attach(Cpu* cpu);
    void Attach(Ppu* ppu);
    void Attach(Apu* apu);
    void Attach(Controller* controller);
    void Attach(std::unique_ptr<Cart> cart);

    void TickCpu();

    uint8_t CpuReadData(uint16_t address);
    uint8_t CpuReadProgramData(uint16_t address);
    void CpuWrite(uint16_t address, uint8_t value);

    uint8_t PpuRead(uint16_t address) const;
    uint16_t PpuReadChr16(uint16_t address) const;
    void PpuWrite(uint16_t address, uint8_t value);

    void SignalNmi();
    void DmaWrite(uint8_t value);

    void OnFrame();

private:
    Cpu* cpu_;
    Ppu* ppu_;
    Apu* apu_;
    Controller* controller_;
    std::unique_ptr<Cart> cart_;

    std::array<uint8_t, 2048> cpuRam_;
    std::array<uint8_t, 2048> ppuRam_;
};