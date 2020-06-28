#pragma once

#include "PpuBackground.h"
#include "PpuSprites.h"

#include <array>
#include <cstdint>

class Bus;
class Display;

class Ppu
{
public:
    Ppu(Bus& bus, Display& display);

    uint32_t FrameCount();

    void Tick3();
    void Sync();
    void SyncA12();

    uint8_t Read(uint16_t address);
    void Write(uint16_t address, uint8_t value);

    void DmaWrite(uint8_t value);

private:
    // this allows the general case of Tick3 to be inlined
    __declspec(noinline) void RunDeferredUpdate();
    __declspec(noinline) void SyncScanline();

    void Sync(int32_t targetCycle);
    void SyncComposite();

    void PreRenderScanline(int32_t targetCycle);
    void RenderScanline(int32_t targetCycle);

    void RenderScanline();

    void Composite(int32_t startCycle, int32_t endCycle);
    void FinishRender();

    void EnterVBlank();
    void SignalVBlank();


    Bus& bus_;
    Display& display_;

    uint32_t frameCount_{};

    bool addressLatch_{};
    uint8_t ppuData_{};

    // PPUCTRL
    bool enableVBlankInterrupt_{};
    uint16_t addressIncrement_{1};

    // PPUMASK
    bool enableBackground_{};
    bool enableForeground_{};
    bool enableRendering_{};

    // PPUSTATUS
    bool inVBlank_{};

    // the bits in the address registers can be viewed as 0yyy NNYY YYYX XXXX
    uint16_t initialAddress_{};

    uint8_t palette_[32];
    uint32_t rgbPalette_[32];

    int32_t currentScanline_{ 0 };
    int32_t scanlineCycle_{ 0 };
    int32_t compositeCycle_{ 0 };
    int32_t targetCycle_{ 0 };

    PpuBackground background_;
    PpuSprites sprites_;

    bool hasDeferredUpdate_{};

    // a 3-cycle delay for updating the background address
    bool updateBaseAddress_{};

    // a 2-cycle delay for updating the PPU masks.
    bool updateMask_{};
    uint8_t mask_{};

    // the next tick should trigger the VBlank interrupt
    bool signalVBlank_{};
};