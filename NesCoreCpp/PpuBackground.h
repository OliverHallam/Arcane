#pragma once

#include <cstdint>

class Bus;

class PpuBackground
{
public:
    PpuBackground(const Bus& bus);

    void SetBasePatternAddress(uint16_t address);

    void SetFineX(uint8_t value);

    int8_t Render();
    void Tick(int32_t scanlineCycle);

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

    Tile loadingTile_{};
    Tile nextTile_{};
    Tile currentTile_{};
    
    // cache for code performance
    int32_t patternBitShift_{};
    uint8_t fineX_{};
    uint16_t backgroundPatternBase_{};
    uint16_t patternAddress_{};

    const Bus& bus_;
};