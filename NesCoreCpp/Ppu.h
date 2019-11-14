#pragma once

#include "PpuBackground.h"
#include "PpuSprites.h"

#include <cstdint>

class Bus;
class Display;

class Ppu
{
public:
    Ppu(Bus& bus, Display& display);

    uint32_t FrameCount();

    void Tick();

    uint8_t Read(uint16_t address);
    void Write(uint16_t address, uint8_t value);

    void DmaWrite(uint8_t value);

private:
    void Sync();

    void RunTickPreRender();
    void RunTickRender();
    void RunTickPostRender();

    Bus& bus_;
    Display& display_;

    // approx!
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

    int32_t currentScanline_{ 0 };
    int32_t scanlineCycle_{ -1 };

    int32_t targetCycle_{ -1 };

    PpuBackground background_;
    PpuSprites sprites_;
};