#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "Cart.h"
#include "Controller.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Apu.h"
#include "EventQueue.h"

class Bus
{
public:
    Bus();

    void Attach(Cpu* cpu);
    void Attach(Ppu* ppu);
    void Attach(Apu* apu);
    void Attach(Controller* controller);
    void Attach(std::unique_ptr<Cart> cart);

    void TickCpuRead();
    void TickCpuWrite();

    uint32_t CycleCount() const;

    void SyncPpu();

    bool SensitiveToChrA12() const;
    void SetChrA12(bool set);

    void CpuDummyRead(uint16_t address);
    uint8_t CpuReadData(uint16_t address);
    uint8_t CpuReadZeroPage(uint16_t address);
    uint8_t CpuReadProgramData(uint16_t address);
    void CpuWrite(uint16_t address, uint8_t value);
    void CpuWriteZeroPage(uint16_t address, uint8_t value);
    void CpuWrite2(uint16_t address, uint8_t firstValue, uint8_t secondValue);

    uint8_t* GetPpuRamBase();
    uint8_t PpuRead(uint16_t address) const;
    uint16_t PpuReadChr16(uint16_t address) const;
    void PpuWrite(uint16_t address, uint8_t value);

    void SignalNmi();
    void SetAudioIrq(bool irq);
    void SetCartIrq(bool irq);

    void BeginOamDma(uint8_t page);
    void BeginDmcDma(uint16_t address);

    void OnFrame();

    void Schedule(uint32_t cycles, SyncEvent evt);
    void RescheduleFrameCounter(uint32_t cycles);

private:
    void Tick();

    __declspec(noinline)
    uint8_t CpuReadProgramDataRare(uint16_t address);

    __declspec(noinline)
    void RunEvent();

    uint8_t DmcDmaRead(uint16_t address);
    void OamDmaWrite(uint8_t value);

    void RunDma();
    void RunOamDma();
    void RunDmcDma();

    Cpu* cpu_;
    Ppu* ppu_;
    Apu* apu_;
    Controller* controller_;
    std::unique_ptr<Cart> cart_;

    std::array<uint8_t, 2048> cpuRam_;
    std::array<uint8_t, 2048> ppuRam_;

    uint32_t cycleCount_;

    bool dma_;
    bool oamDma_;
    bool dmcDma_;

    uint16_t oamDmaAddress_;
    uint16_t dmcDmaAddress_;

    bool audioIrq_;
    bool cartIrq_;

    EventQueue syncQueue_;
};