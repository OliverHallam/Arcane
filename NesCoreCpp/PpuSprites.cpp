#include "Bus.h"
#include "PpuSprites.h"

#include <cstdint>

PpuSprites::PpuSprites(Bus& bus) :
    bus_{ bus }
{
}

void PpuSprites::BasePatternAddress(uint16_t address)
{
    spritePatternBase_ = address;
}

void PpuSprites::OamAddress(uint8_t value)
{
    oamAddress_ = value;
}

void PpuSprites::WriteOam(uint8_t value)
{
    oam_[oamAddress_++] = value;
}

void PpuSprites::EvaluationTick(uint32_t scanline, uint32_t scanlineCycle)
{
    if ((scanlineCycle & 1) == 0)
    {
        // setting up the read/write
        return;
    }

    if (scanlineCycle < 64)
    {
        oamCopy_[scanlineCycle / 2] = oamData_ = 0xff;
        return;
    }

    if (oamAddress_ >= 256)
    {
        return;
    }

    oamData_ = oam_[oamAddress_];
    if (oamCopyIndex_ < 32)
    {
        oamCopy_[oamCopyIndex_] = oamData_;

        if ((oamAddress_ & 0x03) != 0)
        {
            oamAddress_++;
            oamCopyIndex_++;
            return;
        }
    }

    auto spriteRow = (uint32_t)(scanline - oamData_);
    bool visible = spriteRow < 8;

    if (oamAddress_ == 0)
    {
        sprite0Selected_ = visible;
    }

    if (visible)
    {
        if (oamCopyIndex_ >= 32)
        {
            spriteOverflow_ = true;
            oamAddress_ += 4;
        }
        else
        {
            oamAddress_++;
            oamCopyIndex_++;
        }
    }
    else
    {
        oamAddress_ += 4;

        // TODO: overflow bug
    }
}

int8_t PpuSprites::RenderTick(bool pixelRendered)
{
    uint8_t result = 0xff;
    auto drawnSprite = false;

    for (auto i = 0; i < scanlineSpriteCount_; i++)
    {
        if (sprites_[i].X != 0)
        {
            sprites_[i].X--;
            continue;
        }

        uint8_t pixel;
        if ((sprites_[i].attributes & 0x40) == 0)
        {
            pixel = (uint8_t)((sprites_[i].patternShiftHigh & 0x80) >> 6 | (sprites_[i].patternShiftLow & 0x80) >> 7);

            sprites_[i].patternShiftHigh <<= 1;
            sprites_[i].patternShiftLow <<= 1;
        }
        else
        {
            pixel = (uint8_t)((sprites_[i].patternShiftHigh & 0x01) << 1 | sprites_[i].patternShiftLow & 0x01);

            sprites_[i].patternShiftHigh >>= 1;
            sprites_[i].patternShiftLow >>= 1;
        }

        if (pixel != 0 && !drawnSprite)
        {
            drawnSprite = true;

            if (pixelRendered)
            {
                sprite0Hit_ |= i == 0 && sprite0Visible_;

                if ((sprites_[i].attributes & 0x20) != 0)
                {
                    continue;
                }
            }

            pixel |= (uint8_t)((0x04 | (sprites_[i].attributes & 0x03)) << 2); // palette
            result = pixel;
        }
    }

    return result;
}

void PpuSprites::HReset()
{
    scanlineSpriteCount_ = oamCopyIndex_ >> 2;
    sprite0Visible_ = sprite0Selected_;
    sprite0Selected_ = false;
    oamAddress_ = 0;
    oamCopyIndex_ = 0;
    spriteIndex_ = 0;
}

void PpuSprites::VReset()
{
    sprite0Hit_ = false;
}

bool PpuSprites::Sprite0Hit()
{
    return sprite0Hit_;
}

bool PpuSprites::SpriteOverflow()
{
    return spriteOverflow_;
}

void PpuSprites::LoadTick(uint32_t currentScanline, uint32_t scanlineCycle)
{
    if (spriteIndex_ >= scanlineSpriteCount_)
    {
        return;
    }

    switch (scanlineCycle & 0x07)
    {
    case 5:
    {
        auto oamAddress = spriteIndex_ << 2;

        auto attributes = oamCopy_[oamAddress + 2];
        sprites_[spriteIndex_].attributes = attributes;
        sprites_[spriteIndex_].X = oamCopy_[oamAddress + 3];

        auto tileId = oamCopy_[oamAddress + 1];

        auto tileFineY = currentScanline - oamCopy_[oamAddress];

        if ((attributes & 0x80) != 0)
        {
            tileFineY = 7 - tileFineY;
        }

        // address is 000PTTTTTTTT0YYY
        patternAddress_ = (uint16_t)
            (spritePatternBase_ | // pattern selector
            (tileId << 4) |
                tileFineY);
        sprites_[spriteIndex_].patternShiftLow = bus_.PpuRead(patternAddress_);

        break;
    }

    case 7:
        // address is 000PTTTTTTTT1YYY
        sprites_[spriteIndex_].patternShiftHigh = bus_.PpuRead((uint16_t)(patternAddress_ | 8));
        spriteIndex_++;
        break;
    }
}
