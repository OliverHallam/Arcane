#pragma once

#include <array>
#include <cstdint>

class Bus;

class PpuBackground
{
public:
    PpuBackground(const Bus& bus);

    void SetBasePatternAddress(uint16_t address);

    void SetFineX(uint8_t value);

    uint8_t Render();
    void RunLoad(int32_t startCycle, int32_t endCycle);
    void RunLoad();
    void Tick();

    void HReset(uint16_t initialAddress);
    void VReset(uint16_t initialAddress);

    // the bits in the address registers can be viewed as 0yyy NNYY YYYX XXXX
    uint16_t CurrentAddress{};

private:
    struct Tile
    {
        uint8_t PatternByteHigh{};
        uint8_t PatternByteLow{};
        uint8_t AttributeBits{};
    };

    uint8_t nextTileId_{};

    uint32_t loadingIndex_{2};
    std::array<Tile, 34> scanlineTiles_;

    uint32_t currentTileIndex_{};
    Tile currentTile_;

    uint8_t fineX_{};
    int32_t patternBitShift_{};

    // cache for code performance
    uint16_t backgroundPatternBase_{};
    uint16_t patternAddress_{};

    const Bus& bus_;
};