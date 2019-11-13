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
    uint8_t nextTileId_{};
    uint8_t nextPatternByteLow_{};
    uint8_t nextPatternByteHigh_{};

    uint16_t patternShiftHigh_{};
    uint16_t patternShiftLow_{};
    uint32_t attributeShift_{};
    uint16_t nextAttributeShift_{};

    // cache for code performance
    int32_t patternMask_{};
    int32_t patternBitShift_{};
    int32_t attributeMask_{};
    int32_t attributeBitShift_{};
    uint16_t backgroundPatternBase_{};
    uint16_t patternAddress_{};

    const Bus& bus_;
};