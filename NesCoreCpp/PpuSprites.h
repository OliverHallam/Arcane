#pragma once

#include <cstdint>

#include "PpuSpritesState.h"

class Bus;

class PpuSprites
{
public:
    PpuSprites(Bus& bus);

    void SetLargeSprites(bool enabled);
    bool LargeSprites() const;

    void SetBasePatternAddress(uint16_t address);
    uint16_t BasePatternAddress() const;

    void EnableLeftColumn(bool enabled);

    void SetOamAddress(uint8_t value);
    void WriteOam(uint8_t value);
    uint8_t ReadOam() const;

    void OamDmaCompleted();

    void RunEvaluation(uint32_t scanline, uint32_t scanlineCycle, uint32_t targetCycle);

    void HReset();

    void RunRender(uint32_t scanlineCycle, uint32_t targetCycle, const std::array<uint8_t, 256>& backgroundPixels);

    bool SpritesVisible() const;

    bool Sprite0Visible() const;
    bool Sprite0Hit() const;
    bool SpriteOverflow() const;

    bool AllLargeSpritesHighTable() const;
    bool IsHighTable(int32_t spriteIndex) const;

    void RunLoad(uint32_t scanline, uint32_t scanlineCycle, uint32_t targetCycle);
    void RunLoad(uint32_t scanline);
    uint8_t GetLoadingOamData(uint32_t cycle);

    void VReset();

    const std::array<uint8_t, 256>& ScanlineAttributes() const;
    const std::array<uint8_t, 256>& ScanlinePixels() const;

#if DIAGNOSTIC
    void MarkSprites(uint32_t* diagnosticPixels);
#endif

    void CaptureState(PpuSpritesState* state) const;
    void RestoreState(const PpuSpritesState& state);

private:
    struct Sprite
    {
        uint8_t X;
        uint8_t patternShiftHigh;
        uint8_t patternShiftLow;
        uint8_t attributes;
    };

    Bus& bus_;

    PpuSpritesState state_;

    uint16_t patternAddress_{};

    std::array<uint8_t, 32> oamCopy_{};
    uint8_t oamCopyIndex_{};

    int32_t spriteIndex_{};
    std::array<Sprite, 8> sprites_{};
    bool sprite0Selected_{};
    bool sprite0Visible_{};
    int32_t scanlineSpriteCount_{};

    bool allLargeSpritesHighTable_{};

    bool spritesRendered_{};

    std::array<uint8_t, 256> scanlineAttributes_{};
    std::array<uint8_t, 256> scanlineData_{};
};