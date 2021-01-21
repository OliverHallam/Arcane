#pragma once

struct PpuSpritesState
{
    bool largeSprites_{};
    uint16_t spritePatternBase_{};

    uint8_t oamAddress_{};
    uint32_t spriteEvaluationOamAddress_{};
    std::array<uint8_t, 256> oam_{};

    bool sprite0Hit_{};
    bool spriteOverflow_{};

    uint32_t leftCrop_;
};