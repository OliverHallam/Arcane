#pragma once

#include <cstdint>

class Bus;

class PpuSprites
{
public:
    PpuSprites(Bus& bus);

    void SetBasePatternAddress(uint16_t address);

    void SetOamAddress(uint8_t value);
    void WriteOam(uint8_t value);

    void RunEvaluation(uint32_t scanline, uint32_t scanlineCycle, uint32_t targetCycle);

    void HReset();

    void RunRender(uint32_t scanlineCycle, uint32_t targetCycle, const std::array<uint8_t, 256>& backgroundPixels);

    bool SpritesVisible();

    bool Sprite0Visible();
    bool Sprite0Hit();
    bool SpriteOverflow();

    void RunLoad(uint32_t scanline, uint32_t scanlineCycle, uint32_t targetCycle);
    void RunLoad(uint32_t scanline);

    void VReset();

    const std::array<uint8_t, 256>& ScanlineAttributes() const;
    const std::array<uint8_t, 256>& ScanlinePixels() const;

private:
    struct Sprite
    {
        uint8_t X;
        uint8_t patternShiftHigh;
        uint8_t patternShiftLow;
        uint8_t attributes;
    };

    Bus& bus_;

    uint16_t spritePatternBase_{};
    uint16_t patternAddress_{};

    short oamAddress_{};
    uint8_t oamData_{};
    uint8_t oam_[256]{};
    uint8_t oamCopy_[32]{};
    uint8_t oamCopyIndex_{};

    int32_t spriteIndex_{};
    Sprite sprites_[8]{};
    bool sprite0Selected_{};
    bool sprite0Visible_{};
    int32_t scanlineSpriteCount_{};

    bool sprite0Hit_{};
    bool spriteOverflow_{};

    std::array<uint8_t, 256> scanlineAttributes_{};
    std::array<uint8_t, 256> scanlineData_{};
};