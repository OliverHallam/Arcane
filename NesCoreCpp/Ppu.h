#pragma once

#include "ChrA12.h"
#include "PpuBackground.h"
#include "PpuSprites.h"
#include "PpuCoreState.h"
#include <array>
#include <cstdint>

class Bus;
class Display;
struct PpuState;

class Ppu
{
public:
    Ppu(Bus& bus, Display& display);

    uint32_t FrameCount();

    void Sync();
    void SyncA12();
    void SyncScanline();
    void SyncState();

    uint8_t Read(uint16_t address);
    void Write(uint16_t address, uint8_t value);

    void DmaWrite(uint8_t value);
    void DmaCompleted();

    void CaptureState(PpuState* state) const;
    void RestoreState(const PpuState& state);

    void UpdateA12Sensitivity(bool isLow);

#ifdef DIAGNOSTIC
    void MarkDiagnostic(uint32_t color);
#endif

private:
    int32_t ScanlineCycle() const;

    void Sync(int32_t targetCycle);
    void SyncComposite(int32_t targetCycle);

    void PreRenderScanline(int32_t targetCycle);
    void RenderScanline(int32_t targetCycle);
    void RenderScanlineVisible(int32_t targetCycle);

    void RenderScanline();
    void RenderScanlineVisible();

    void Composite(int32_t startCycle, int32_t endCycle);
    void FinishRender();

#ifdef DIAGNOSTIC
    void Clear(int32_t scanline);
    void RenderOverlay(int32_t scanline);
#endif

    void EnterVBlank();

    void SetCurrentAddress(uint16_t address);

    void ScheduleA12Sync(int32_t cycle, bool isLow);

    Bus& bus_;
    Display& display_;

    ChrA12 chrA12_;

    PpuBackground background_;
    PpuSprites sprites_;

    PpuCoreState state_;


#if DIAGNOSTIC
    std::array<uint32_t, 341> diagnosticOverlay_{};
#endif
};