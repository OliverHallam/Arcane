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
    Bus& bus_;
    Display& display_;

    // approx!
    uint32_t frameCount_{};

    uint8_t ppuStatus_{};
    uint8_t ppuControl_{};
    uint8_t ppuMask_{};

    bool addressLatch_{};
    uint8_t ppuData_{};

    // the bits in the address registers can be viewed as 0yyy NNYY YYYX XXXX
    uint16_t initialAddress_{};

    uint8_t palette_[32];

    int32_t currentScanline_{ -1 };
    int32_t scanlineCycle_{ -1 };

    PpuBackground background_;
    PpuSprites sprites_;
};