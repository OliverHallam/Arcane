#pragma once

struct PpuSpritesState
{
    bool largeSprites_{};
    uint16_t spritePatternBase_{};

    uint32_t oamAddress_{};
    std::array<uint8_t, 256> oam_{};

    bool sprite0Hit_{};
    bool spriteOverflow_{};

    uint32_t leftCrop_;
};