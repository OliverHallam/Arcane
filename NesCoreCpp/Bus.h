#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "BusState.h"
#include "Cart.h"
#include "ChrA12Sensitivity.h"
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
    void AttachPlayer1(Controller* controller);
    void AttachPlayer2(Controller* controller);
    void Attach(Cart* cart);

    void DetachCart();

    bool HasCart() const;

    __forceinline void TickCpuRead();
    __forceinline void TickCpuWrite();

    uint32_t CpuCycleCount() const;
    uint32_t PpuCycleCount() const;

    int32_t PpuScanlineCycle() const;

    void SyncPpu();

    ChrA12Sensitivity ChrA12Sensitivity() const;
    void UpdateA12Sensitivity();
    uint32_t GetChrA12PulsesRequiredForSync() const;
    void ChrA12Rising();
    void ChrA12Falling();
    uint32_t GetA12FallingEdgeCycleSmoothed() const;

    bool HasScanlineCounter() const;
    void TileSplitBeginScanline();
    void TileSplitBeginTile(uint32_t index);
    void TileSplitBeginSprites();
    void TileSplitEndSprites();

    void CpuDummyRead(uint16_t address);
    uint8_t CpuReadData(uint16_t address);
    uint8_t CpuReadZeroPage(uint16_t address);
    uint8_t CpuReadProgramData(uint16_t address);
    void CpuWrite(uint16_t address, uint8_t value);
    void CpuWriteZeroPage(uint16_t address, uint8_t value);
    void CpuWrite2(uint16_t address, uint8_t firstValue, uint8_t secondValue);

    uint8_t* GetPpuRamBase();
    uint8_t PpuRead(uint16_t address) const;
    uint8_t PpuReadNametable(uint16_t address) const;
    uint8_t PpuReadAttributes(uint16_t address) const;
    uint8_t PpuReadPatternLow(uint16_t address) const;
    uint8_t PpuReadPatternHigh(uint16_t address) const;
    uint8_t PpuReadSpritePatternLow(uint16_t address) const;
    uint8_t PpuReadSpritePatternHigh(uint16_t address) const;
    uint16_t PpuReadPattern16(uint16_t address) const;
    void PpuWrite(uint16_t address, uint8_t value);

    void InterceptPpuCtrl(bool largeSprites);
    void InterceptPpuMask(bool renderingEnabled);
    bool PpuIsRendering();

    void WriteMMC5Audio(uint16_t address, uint8_t value);

    void SignalNmi();
    void SetAudioIrq(bool irq);
    void SetCartIrq(bool irq);

    void BeginOamDma(uint8_t page);
    void BeginDmcDma(uint16_t address);
    void CancelDmcDma();

    void OnFrame();

    void Schedule(uint32_t cycles, SyncEvent evt);
    void SchedulePpu(uint32_t cycles, SyncEvent evt);
    bool Deschedule(SyncEvent evt);
    bool DescheduleAll(SyncEvent evt);

    void CaptureState(BusState* state) const;
    void RestoreState(const BusState& state);

#ifdef DIAGNOSTIC
    void MarkDiagnostic(uint32_t color);
#endif

private:
    __forceinline void Tick();

    uint8_t CpuReadImpl(uint16_t address);

    __declspec(noinline)
    uint8_t CpuReadProgramDataRare(uint16_t address);

    __declspec(noinline)
    void RunEvent();

    uint8_t OamDmaRead(uint16_t address);
    uint8_t DmcDmaRead(uint16_t address);
    void OamDmaWrite(uint8_t value);

    void RunDma();
    void RunOamDma();
    void RunDmcDma();

    Cpu* cpu_;
    Ppu* ppu_;
    Apu* apu_;
    Controller* controller1_;
    Controller* controller2_;
    Cart* cart_;

    BusState state_;
};