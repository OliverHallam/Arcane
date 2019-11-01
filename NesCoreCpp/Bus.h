#pragma once

#include <array>
#include <cstdint>
#include <memory>

class Cart;
class Controller;
class Cpu;
class Ppu;

class Bus
{
public:
    Bus();

    void Attach(std::unique_ptr<Cpu> cpu);
    void Attach(std::unique_ptr<Ppu> ppu);
    void Attach(std::unique_ptr<Controller> controller);
    void Attach(std::unique_ptr<Cart> cart);

    void TickCpu();

    uint8_t CpuReadData(uint16_t address);
    uint8_t CpuReadProgramData(uint16_t address);
    void CpuWrite(uint16_t address, uint8_t value);

    uint8_t PpuRead(uint16_t address);
    void PpuWrite(uint16_t address, uint8_t value);

    void SignalNmi();
    void DmaWrite(uint8_t value);

private:
    std::unique_ptr<Cpu> cpu_;
    std::unique_ptr<Ppu> ppu_;
    std::unique_ptr<Controller> controller_;
    std::unique_ptr<Cart> cart_;

    std::array<uint8_t, 2048> cpuRam_;
    std::array<uint8_t, 2048> ppuRam_;
};