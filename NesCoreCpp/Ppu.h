#pragma once

#include <cstdint>

class Bus;
class Display;

class Ppu
{
public:
    Ppu(Bus& bus, Display& display);

    void Tick();

    uint8_t Read(uint16_t address);
    void Write(uint16_t address, uint8_t value);

    void DmaWrite(uint8_t value);

private:
    struct Sprite
    {
        uint8_t X;
        uint8_t patternShiftHigh;
        uint8_t patternShiftLow;
        uint8_t attributes;
    };

    void SetFineX(uint8_t value);

    void BackgroundRender();
    void BackgroundLoadTick();
    void BackgroundTick();
    void BackgroundVReset();
    void BackgroundHReset();

    void SpriteRender();
    void SpriteTick();
    void SpriteEvaluationTick();
    void SpriteLoadTick();

    Bus& bus_;
    Display& display_;

    // approx!
    int32_t frameCount_{};

    uint8_t ppuStatus_{};
    uint8_t ppuControl_{};
    uint8_t ppuMask_{};

    bool addressLatch_{};

    // the bits in the address registers can be viewed as 0yyy NNYY YYYX XXXX
    uint16_t currentAddress_{};
    uint16_t initialAddress_{};

    uint8_t ppuData_{};

    uint8_t palette_[32];

    int32_t currentScanLine_{ -1 };
    int32_t scanlineCycle_{ -1 };

    uint8_t nextTileId_{};
    uint8_t nextPatternByteLow_{};
    uint8_t nextPatternByteHigh_{};

    uint16_t patternShiftHigh_{};
    uint16_t patternShiftLow_{};
    uint32_t attributeShift_{};
    uint16_t nextAttributeShift_{};
    uint8_t currentPixel_{};
    bool pixelRendered_{};

    // cache for code performance
    int32_t patternMask_{};
    int32_t patternBitShift_{};
    int32_t attributeMask_{};
    int32_t attributeBitShift_{};
    uint16_t backgroundPatternBase_{};
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
};