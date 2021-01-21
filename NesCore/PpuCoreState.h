#pragma once

#include <cstdint>

struct PpuCoreState
{
    uint32_t FrameCount{};

    bool AddressLatch{};
    uint8_t PpuData{};

    // PPUCTRL
    bool EnableVBlankInterrupt{};
    uint16_t AddressIncrement{ 1 };

    // PPUMASK
    bool EnableBackground{};
    bool EnableForeground{};
    bool EnableRendering{};

    // PPUSTATUS
    bool SuppressVBlank{};
    bool InVBlank{};

    // the bits in the address registers can be viewed as 0yyy NNYY YYYX XXXX
    uint16_t InitialAddress{};

    uint8_t GrayscaleMask{ 0xff };
    uint8_t Emphasis{ 0 };
    uint8_t Palette[32]{ };
    // TODO: this can be recomputed when we restore state
    uint32_t RgbPalette[32]{ };

    // TODO: we don't need all these fields in our stored state.
    int32_t CurrentScanline{ 0 };
    uint32_t ScanlineStartCycle{ 0 };
    int32_t SyncCycle{ 0 };
    int32_t CompositeCycle{ 0 };
    int32_t ChrA12Cycle{ 0 };

    // a 3-cycle delay for updating the background address
    bool UpdateBaseAddress{};

    // a 2-cycle delay for updating the PPU masks.
    bool UpdateMask{};
    uint8_t Mask{};
};