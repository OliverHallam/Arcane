#include "Bus.h"
#include "PpuSprites.h"

#include <cstdint>

PpuSprites::PpuSprites(Bus& bus) :
    bus_{ bus }
{
}

void PpuSprites::SetBasePatternAddress(uint16_t address)
{
    spritePatternBase_ = address;
}

void PpuSprites::SetOamAddress(uint8_t value)
{
    oamAddress_ = value;
}

void PpuSprites::WriteOam(uint8_t value)
{
    oam_[oamAddress_++] = value;
}

void PpuSprites::RunEvaluation(uint32_t scanline, uint32_t scanlineCycle, uint32_t targetCycle)
{
    if (scanlineCycle < 64)
    {
        if (64 >= targetCycle)
            return;
    }

    if ((scanlineCycle & 1) == 0)
    {
        // setting up the read/write
        scanlineCycle++;
    }

    while (scanlineCycle < targetCycle)
    {
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
                scanlineCycle += 2;
                continue;
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

        scanlineCycle += 2;
    }
}

void PpuSprites::RunRender(uint32_t scanlineCycle, uint32_t targetCycle, const std::array<uint8_t, 256>& backgroundPixels)
{
    // evaluate the sprites backwards to overwrite them in the right order
    for (auto spriteIndex = scanlineSpriteCount_ - 1; spriteIndex >= 0; spriteIndex--)
    {
        auto& sprite = sprites_[spriteIndex];
        auto startX = std::max((uint32_t)sprite.X, scanlineCycle);
        auto endX = std::min((uint32_t)sprite.X + 8, targetCycle);

        for (auto cycle = startX; cycle < endX; cycle++)
        {
            uint8_t pixel;
            if ((sprite.attributes & 0x40) == 0)
            {
                pixel = (uint8_t)((sprite.patternShiftHigh & 0x80) >> 6 | (sprite.patternShiftLow & 0x80) >> 7);

                sprite.patternShiftHigh <<= 1;
                sprite.patternShiftLow <<= 1;
            }
            else
            {
                pixel = (uint8_t)((sprite.patternShiftHigh & 0x01) << 1 | sprite.patternShiftLow & 0x01);

                sprite.patternShiftHigh >>= 1;
                sprite.patternShiftLow >>= 1;
            }

            if (pixel != 0)
            {
                if (spriteIndex == 0 && sprite0Visible_ && backgroundPixels[cycle])
                {
                    sprite0Hit_ = true;
                }

                pixel |= (uint8_t)((0x04 | (sprite.attributes & 0x03)) << 2); // palette

                scanlineAttributes_[cycle] = sprite.attributes;
                scanlineData_[cycle] = pixel;
            }
        }
    }
}

void PpuSprites::HReset()
{
    scanlineSpriteCount_ = oamCopyIndex_ >> 2;
    sprite0Visible_ = sprite0Selected_;
    sprite0Selected_ = false;
    oamAddress_ = 0;
    oamCopyIndex_ = 0;
    spriteIndex_ = 0;
    scanlineData_.fill(0);
    scanlineAttributes_.fill(0);
}

void PpuSprites::VReset()
{
    sprite0Hit_ = false;
}

const std::array<uint8_t, 256>& PpuSprites::ScanlineAttributes() const
{
    return scanlineAttributes_;
}

const std::array<uint8_t, 256>& PpuSprites::ScanlinePixels() const
{
    return scanlineData_;
}

bool PpuSprites::Sprite0Hit()
{
    return sprite0Hit_;
}

bool PpuSprites::SpriteOverflow()
{
    return spriteOverflow_;
}

void PpuSprites::RunLoad(uint32_t currentScanline, uint32_t scanlineCycle, uint32_t targetCycle)
{
    // TODO: should be 321?
    if (scanlineCycle == 257 && targetCycle == 320)
    {
        RunLoad(currentScanline);
        return;
    }

    if (spriteIndex_ >= scanlineSpriteCount_)
    {
        return;
    }

    switch (scanlineCycle & 0x07)
    {
        while (true)
        {
    case 0:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                return;

    case 1:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                return;

    case 2:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                return;

    case 3:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                return;

    case 4:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                return;

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
                    (tileId << 4) | tileFineY);
                sprites_[spriteIndex_].patternShiftLow = bus_.PpuRead(patternAddress_);
            }

            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                return;

    case 6:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                return;

    case 7:
            // address is 000PTTTTTTTT1YYY
            sprites_[spriteIndex_].patternShiftHigh = bus_.PpuRead((uint16_t)(patternAddress_ | 8));
            spriteIndex_++;

            if (spriteIndex_ >= scanlineSpriteCount_)
                return;

            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                return;

        }
    }
}

void PpuSprites::RunLoad(uint32_t currentScanline)
{
    while (spriteIndex_ < scanlineSpriteCount_)
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
            (tileId << 4) | tileFineY);
        sprites_[spriteIndex_].patternShiftLow = bus_.PpuRead(patternAddress_);

        // address is 000PTTTTTTTT1YYY
        sprites_[spriteIndex_].patternShiftHigh = bus_.PpuRead((uint16_t)(patternAddress_ | 8));
        spriteIndex_++;
    }
}
