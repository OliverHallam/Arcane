#include "Bus.h"
#include "PpuSprites.h"

#include <cassert>
#include <cstdint>

PpuSprites::PpuSprites(Bus& bus) :
    bus_{ bus }
{
}

void PpuSprites::SetLargeSprites(bool enabled)
{
    state_.largeSprites_ = enabled;
}

bool PpuSprites::LargeSprites() const
{
    return state_.largeSprites_;
}

void PpuSprites::SetBasePatternAddress(uint16_t address)
{
    state_.spritePatternBase_ = address;
}

uint16_t PpuSprites::BasePatternAddress() const
{
    return state_.spritePatternBase_;
}

void PpuSprites::EnableLeftColumn(bool enabled)
{
    state_.leftCrop_ = enabled ? 0 : 8;
}

void PpuSprites::SetOamAddress(uint8_t value)
{
    state_.oamAddress_ = value;
}

void PpuSprites::WriteOam(uint8_t value)
{
    state_.oam_[state_.oamAddress_++] = value;
}

uint8_t PpuSprites::ReadOam() const
{
    return state_.oam_[state_.oamAddress_];
}

void PpuSprites::OamDmaCompleted()
{
    allLargeSpritesHighTable_ = true;

    for (auto oamAddress = 0; oamAddress < 256; oamAddress += 4)
    {
        auto tileId = state_.oam_[oamAddress + 1];
        allLargeSpritesHighTable_ &= tileId & 1;
    }

}

void PpuSprites::RunEvaluation(uint32_t scanline, uint32_t scanlineCycle, uint32_t targetCycle)
{
    if (state_.oamAddress_ >= 256)
        return;

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

    auto spriteSize = state_.largeSprites_ ? 16u : 8u;

    while (scanlineCycle < targetCycle)
    {
        auto oamData = state_.oam_[state_.oamAddress_];
        if (oamCopyIndex_ < 32)
        {
            oamCopy_[oamCopyIndex_] = oamData;

            if ((state_.oamAddress_ & 0x03) != 0)
            {
                state_.oamAddress_++;
                oamCopyIndex_++;
                scanlineCycle += 2;
                continue;
            }
        }

        auto spriteRow = (uint32_t)(scanline - oamData);
        bool visible = spriteRow < spriteSize;

        if (state_.oamAddress_ == 0)
        {
            sprite0Selected_ = visible;
        }

        if (visible)
        {
            if (oamCopyIndex_ >= 32)
            {
                state_.spriteOverflow_ = true;

                // no more ovserverble side effects
                state_.oamAddress_ == 256;
                return;
            }
            else
            {
                state_.oamAddress_++;
                oamCopyIndex_++;
            }
        }
        else
        {
            state_.oamAddress_ += 4;

            // TODO: overflow bug
        }

        scanlineCycle += 2;
    }
}

void PpuSprites::RunEvaluation(uint32_t scanline)
{
    auto spriteSize = state_.largeSprites_ ? 16u : 8u;

    auto oamAddress = 0;
    auto oamCopyIndex = 0;

    while (oamAddress < 256)
    {
        auto oamData = state_.oam_[oamAddress];
        auto spriteRow = (uint32_t)(scanline - oamData);
        bool visible = spriteRow < spriteSize;

        if (oamAddress == 0)
        {
            sprite0Selected_ = visible;
        }

        if (visible)
        {
            if (oamCopyIndex >= 32)
            {
                state_.spriteOverflow_ = true;
                return;
            }
            else
            {
                oamCopy_[oamCopyIndex++] = state_.oam_[oamAddress++];
                oamCopy_[oamCopyIndex++] = state_.oam_[oamAddress++];
                oamCopy_[oamCopyIndex++] = state_.oam_[oamAddress++];
                oamCopy_[oamCopyIndex++] = state_.oam_[oamAddress++];
            }
        }
        else
        {
            oamAddress += 4;
            // TODO: overflow bug
        }
    }

    oamCopyIndex_ = oamCopyIndex;
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
                    state_.sprite0Hit_ = true;
                }

                pixel |= (uint8_t)((0x04 | (sprite.attributes & 0x03)) << 2); // palette

                scanlineAttributes_[cycle] = sprite.attributes;
                scanlineData_[cycle] = pixel;
            }
        }
    }

    // TODO: we may be able to do this earlier but would have to mess with the shifters.
    // it's easier to mask it off at the end!
    if (scanlineCycle < state_.leftCrop_)
    {
        auto cropMax = std::min(state_.leftCrop_, targetCycle);
        for (auto cycle = 0u; cycle < cropMax; cycle++)
        {
            scanlineData_[cycle] = 0;
        }
    }
}

bool PpuSprites::SpritesVisible() const
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
    state_.sprite0Hit_ = false;
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

void PpuSprites::CaptureState(PpuSpritesState* state) const
{
    *state = state_;
}

void PpuSprites::RestoreState(const PpuSpritesState& state)
{
    state_ = state;
}

bool PpuSprites::Sprite0Visible() const
{
    return sprite0Visible_;
}

bool PpuSprites::Sprite0Hit() const
{
    return state_.sprite0Hit_;
}

bool PpuSprites::SpriteOverflow() const
{
    return state_.spriteOverflow_;
}

bool PpuSprites::AllLargeSpritesHighTable() const
{
    return allLargeSpritesHighTable_;
}

bool PpuSprites::IsHighTable(int32_t spriteIndex) const
{
    if (spriteIndex >= scanlineSpriteCount_)
        return true;

    auto oamAddress = static_cast<size_t>(spriteIndex) << 2;
    auto tileId = oamCopy_[oamAddress + 1];
    return tileId & 1;
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
    state_.oamAddress_ = 0;

    if (spriteIndex_ >= scanlineSpriteCount_)
    {
        while (spriteIndex_ < 8)
        {
            // no harm in this getting ahead since it only affects the PPU
            bus_.PpuDummyTileFetch();
            spriteIndex_++;
        }

        return;
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
            if (spriteIndex_ < scanlineSpriteCount_)
                bus_.PpuDummyNametableFetch();

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
                    while (spriteIndex_ < 8)
                    {
                        // no harm in this getting ahead since it only affects the PPU
                        bus_.PpuDummyTileFetch();
                        spriteIndex_++;
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
                    if (state_.largeSprites_)
                        tileFineY = 15 - tileFineY;
                    else
                        tileFineY = 7 - tileFineY;
                }

                if (state_.largeSprites_)
                {
                    // address is 000PTTTTTTTY0YYY
                    auto bankAddress = (tileId & 1) << 12;

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
                        (state_.spritePatternBase_ | // pattern selector
                            (tileId << 4) | tileFineY);
                }

                sprites_[spriteIndex_].patternShiftLow = bus_.PpuReadPatternLow(patternAddress_);
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
            sprites_[spriteIndex_].patternShiftHigh = bus_.PpuReadPatternHigh((uint16_t)(patternAddress_ | 8));
            spriteIndex_++;

            if (spriteIndex_ >= scanlineSpriteCount_)
            {
                if (!state_.largeSprites_ || scanlineSpriteCount_ == 8)
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
    state_.oamAddress_ = 0;

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
            if (state_.largeSprites_)
                tileFineY = 15 - tileFineY;
            else
                tileFineY = 7 - tileFineY;
        }

        if (state_.largeSprites_)
        {
            // address is 000PTTTTTTTY0YYY
            auto bankAddress = (tileId & 1) << 12;
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
                (state_.spritePatternBase_ | // pattern selector
                    (tileId << 4) | tileFineY);
        }

        bus_.PpuDummyNametableFetch();

        // TODO: PpuReadPattern16?
        sprites_[spriteIndex_].patternShiftLow = bus_.PpuReadPatternLow(patternAddress_);

        // address is 000PTTTTTTTT1YYY
        sprites_[spriteIndex_].patternShiftHigh = bus_.PpuReadPatternHigh((uint16_t)(patternAddress_ | 8));
        spriteIndex_++;
    }

    if (scanlineSpriteCount_ < 8)
    {
        while (spriteIndex_ < 8)
        {
            bus_.PpuDummyTileFetch();
            spriteIndex_++;
        }
    }
}

uint8_t PpuSprites::GetLoadingOamData(uint32_t cycle)
{
    // we spend 8 cycles loading each sprite
    auto spriteIndex = (cycle - 256) >> 3;
    auto tileCycle = cycle & 7;

    if (static_cast<int32_t>(spriteIndex) > scanlineSpriteCount_)
        return 0xff;

    auto byteIndex = tileCycle > 3 ? 3 : tileCycle;
    return oamCopy_[(spriteIndex << 2) | byteIndex];
}
