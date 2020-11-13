#include "Bus.h"
#include "PpuSprites.h"

#include <cstdint>

PpuSprites::PpuSprites(Bus& bus) :
    bus_{ bus }
{
}

void PpuSprites::SetLargeSprites(bool enabled)
{
    largeSprites_ = enabled;
}

bool PpuSprites::LargeSprites() const
{
    return largeSprites_;
}

void PpuSprites::SetBasePatternAddress(uint16_t address)
{
    spritePatternBase_ = address;
}

uint16_t PpuSprites::BasePatternAddress() const
{
    return spritePatternBase_;
}

void PpuSprites::EnableLeftColumn(bool enabled)
{
    leftCrop_ = enabled ? 0 : 8;
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

    auto spriteSize = largeSprites_ ? 16u : 8u;

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
        bool visible = spriteRow < spriteSize;

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

        if (sprite.patternShiftLow == 0 && sprite.patternShiftHigh == 0)
        {
            continue;
        }

        spritesRendered_ = true;

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
                    // TODO: one pixel early?
                    sprite0Hit_ = true;
                }

                pixel |= (uint8_t)((0x04 | (sprite.attributes & 0x03)) << 2); // palette

                scanlineAttributes_[cycle] = sprite.attributes;
                scanlineData_[cycle] = pixel;
            }
        }
    }

    // TODO: we may be able to do this earlier but would have to mess with the shifters.
    // it's easier to mask it off at the end!
    if (scanlineCycle < leftCrop_)
    {
        auto cropMax = std::min(leftCrop_, targetCycle);
        for (auto cycle = 0u; cycle < cropMax; cycle++)
        {
            scanlineData_[cycle] = 0;
        }
    }
}

bool PpuSprites::SpritesVisible()
{
    return spritesRendered_;
}

void PpuSprites::HReset()
{
    scanlineSpriteCount_ = oamCopyIndex_ >> 2;
    sprite0Visible_ = sprite0Selected_;
    sprite0Selected_ = false;
    oamCopyIndex_ = 0;
    spriteIndex_ = 0;
    spritesRendered_ = false;

    if (scanlineSpriteCount_ > 0)
    {
        scanlineData_.fill(0);
        scanlineAttributes_.fill(0);
    }
}

void PpuSprites::VReset()
{
    sprite0Hit_ = false;
    spritesRendered_ = false;
}

const std::array<uint8_t, 256>& PpuSprites::ScanlineAttributes() const
{
    return scanlineAttributes_;
}

const std::array<uint8_t, 256>& PpuSprites::ScanlinePixels() const
{
    return scanlineData_;
}

#if DIAGNOSTIC
void PpuSprites::MarkSprites(uint32_t* diagnosticPixels)
{
    for (auto i = 0; i < scanlineSpriteCount_; i++)
    {
        diagnosticPixels[sprites_[i].X] = 0xff00ff00;
    }
}
#endif

bool PpuSprites::Sprite0Visible()
{
    return sprite0Visible_;
}

bool PpuSprites::Sprite0Hit()
{
    return sprite0Hit_;
}

bool PpuSprites::SpriteOverflow()
{
    return spriteOverflow_;
}

void PpuSprites::DummyLoad()
{
    bus_.SetChrA12(largeSprites_ || spritePatternBase_ != 0);
}

void PpuSprites::RunLoad(uint32_t currentScanline, uint32_t scanlineCycle, uint32_t targetCycle)
{
    // TODO: should be 321?
    if (scanlineCycle == 256 && targetCycle == 320)
    {
        RunLoad(currentScanline);
        return;
    }

    // the OAM address is forced to 0 during the whole load phase.
    oamAddress_ = 0;

    if (spriteIndex_ >= scanlineSpriteCount_)
    {
        bus_.SetChrA12(largeSprites_ || spritePatternBase_ != 0);
        return;
    }

    if (!largeSprites_)
        bus_.SetChrA12(spritePatternBase_ != 0);
    else
    {
        if (targetCycle < 262)
        {
            // TODO: we may need to be really precise exactly at what point in each CPU cycle the memory access happens to schedule this right!
            // come back when we load the first sprite
            bus_.Schedule((262 - targetCycle + 2) / 3, SyncEvent::PpuSync);
        }
    }
    

    switch (scanlineCycle & 0x07)
    {
        while (true)
        {
    case 0:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                break;

    case 1:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                break;

    case 2:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                break;

    case 3:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                return;

    case 4:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                break;

    case 5:
            {
                if (spriteIndex_ >= scanlineSpriteCount_)
                {
                    if (largeSprites_ && scanlineSpriteCount_ < 8)
                    {
                        bus_.SetChrA12(true);
                    }
                    return;
                }

                auto oamAddress = static_cast<size_t>(spriteIndex_) << 2;

                auto attributes = oamCopy_[oamAddress + 2];
                sprites_[spriteIndex_].attributes = attributes;
                sprites_[spriteIndex_].X = oamCopy_[oamAddress + 3];

                auto tileId = oamCopy_[oamAddress + 1];

                auto tileFineY = currentScanline - oamCopy_[oamAddress];

                if ((attributes & 0x80) != 0)
                {
                    if (largeSprites_)
                        tileFineY = 15 - tileFineY;
                    else
                        tileFineY = 7 - tileFineY;
                }

                if (largeSprites_)
                {
                    // address is 000PTTTTTTTY0YYY
                    auto bankAddress = (tileId & 1) << 12;
                    // TODO: this should happen one cycle sooner
                    bus_.SetChrA12(bankAddress != 0);

                    tileId &= 0xfe;
                    tileId |= tileFineY >> 3;
                    tileFineY &= 0x07;
                    patternAddress_ = (uint16_t)
                        (bankAddress | (tileId << 4) | tileFineY);

                    if (bus_.SensitiveToChrA12())
                    {
                        if (targetCycle <= scanlineCycle + 8)
                        {
                            // we'll need to sync when we read the next sprite
                            bus_.Schedule((scanlineCycle + 9 - targetCycle + 2) / 3, SyncEvent::PpuSync);
                        }
                    }
                }
                else
                {
                    // address is 000PTTTTTTTT0YYY
                    patternAddress_ = (uint16_t)
                        (spritePatternBase_ | // pattern selector
                            (tileId << 4) | tileFineY);
                }

                sprites_[spriteIndex_].patternShiftLow = bus_.PpuRead(patternAddress_);
            }


            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                break;

    case 6:
            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                break;

    case 7:
            // address is 000PTTTTTTTT1YYY
            sprites_[spriteIndex_].patternShiftHigh = bus_.PpuRead((uint16_t)(patternAddress_ | 8));
            spriteIndex_++;

            if (spriteIndex_ >= scanlineSpriteCount_)
            {
                if (!largeSprites_ || scanlineSpriteCount_ == 8)
                {
                    return;
                }
            }

            scanlineCycle++;
            if (scanlineCycle >= targetCycle)
                break;
        }
    }

}

void PpuSprites::RunLoad(uint32_t currentScanline)
{
    // the OAM address is forced to 0 during the whole load phase.
    oamAddress_ = 0;

    if (!largeSprites_)
        bus_.SetChrA12(spritePatternBase_ != 0);

    while (spriteIndex_ < scanlineSpriteCount_)
    {
        auto oamAddress = static_cast<size_t>(spriteIndex_) << 2;

        auto attributes = oamCopy_[oamAddress + 2];
        sprites_[spriteIndex_].attributes = attributes;
        sprites_[spriteIndex_].X = oamCopy_[oamAddress + 3];

        auto tileId = oamCopy_[oamAddress + 1];

        auto tileFineY = currentScanline - oamCopy_[oamAddress];

        if ((attributes & 0x80) != 0)
        {
            if (largeSprites_)
                tileFineY = 15 - tileFineY;
            else
                tileFineY = 7 - tileFineY;
        }

        if (largeSprites_)
        {
            // address is 000PTTTTTTTY0YYY
            auto bankAddress = (tileId & 1) << 12;
            bus_.SetChrA12(bankAddress != 0);

            tileId &= 0xfe;
            tileId |= tileFineY >> 3;
            tileFineY &= 0x07;
            patternAddress_ = (uint16_t)
                (bankAddress | (tileId << 4) | tileFineY);
        }
        else
        {
            // address is 000PTTTTTTTT0YYY
            patternAddress_ = (uint16_t)
                (spritePatternBase_ | // pattern selector
                    (tileId << 4) | tileFineY);
        }

        sprites_[spriteIndex_].patternShiftLow = bus_.PpuRead(patternAddress_);

        // address is 000PTTTTTTTT1YYY
        sprites_[spriteIndex_].patternShiftHigh = bus_.PpuRead((uint16_t)(patternAddress_ | 8));
        spriteIndex_++;
    }

    if (scanlineSpriteCount_ < 8 && largeSprites_)
        bus_.SetChrA12(true);
}
