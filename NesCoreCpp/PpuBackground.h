#pragma once

#include <array>
#include <cstdint>

#include "PpuBackgroundState.h"

class Bus;

class PpuBackground
{
public:
    PpuBackground(Bus& bus);

    void SetBasePatternAddress(uint16_t address);

    void SetFineX(uint8_t value);

    void EnableLeftColumn(bool enabled);

    void EnableRendering(int32_t cycle);

    void BeginScanline();

    uint8_t Render();
    void RunRender(uint32_t startCycle, uint32_t endCycle);
    void RunRenderDisabled(uint32_t startCycle, uint32_t endCycle);

    void RenderScanline();

    void RunLoad(int32_t startCycle, int32_t endCycle);
    void RunLoad();
    void Tick(int32_t cycle);

    void HReset(uint16_t initialAddress);
    void HResetRenderDisabled();
    void VReset(uint16_t initialAddress);

    // the bits in the address registers can be viewed as 0yyy NNYY YYYX XXXX
    uint16_t& CurrentAddress();
    
    const std::array<uint8_t, 256>& ScanlinePixels() const;

    void CaptureState(PpuBackgroundState* state) const;
    void RestoreState(const PpuBackgroundState& state);

private:
    struct alignas(uint32_t) Tile
    {
        uint16_t PatternBytes{};
        uint8_t AttributeBits{};

        bool operator == (const Tile& other)
        {
            return *reinterpret_cast<const uint32_t*>(this) ==
                *reinterpret_cast<const uint32_t*>(&other);
        }

        bool operator != (const Tile& other)
        {
            return !(*this == other);
        }

    private:
        uint8_t Padding{};
    };

    Bus& bus_;

    PpuBackgroundState state_;

    uint8_t nextTileId_{};

    uint32_t loadingIndex_{2};

    std::array<Tile, 34> scanlineTiles_;

    uint32_t currentTileIndex_{};
    Tile currentTile_;

    int32_t patternBitShift_{7};

    // cache for code performance
    uint16_t patternAddress_{};

    std::array<uint8_t, 256> backgroundPixels_{};
};