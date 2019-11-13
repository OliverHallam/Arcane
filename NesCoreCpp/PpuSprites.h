#pragma once

#include <cstdint>

class Bus;

class PpuSprites
{
public:
    PpuSprites(Bus& bus);

    void BasePatternAddress(uint16_t address);

    void OamAddress(uint8_t value);
    void WriteOam(uint8_t value);

    void EvaluationTick(uint32_t scanline, uint32_t scanlineCycle);

    void HReset();

    int8_t RenderTick(bool pixelRendered);

    bool Sprite0Hit();
    bool SpriteOverflow();

    void LoadTick(uint32_t scanline, uint32_t scanlineCycle);

    void VReset();

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
};