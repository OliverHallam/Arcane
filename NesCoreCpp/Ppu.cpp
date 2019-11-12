#include "Ppu.h"

#include "Bus.h"
#include "Display.h"

Ppu::Ppu(Bus& bus, Display& display) :
    bus_{ bus },
    display_{ display }
{
}

uint32_t Ppu::FrameCount()
{
    return frameCount_;
}

void Ppu::Tick()
{
    if (scanlineCycle_ == 0)
    {
        if (currentScanLine_ == 241)
        {
            display_.VBlank();

            frameCount_++;

            if ((ppuControl_ & 0x80) != 0)
            {
                bus_.SignalNmi();
            }

            // The VBlank flag of the PPU is set at tick 1 (the second tick) of scanline 241
            ppuStatus_ |= 0x80;
        }
        else if (currentScanLine_ == -1)
        {
            ppuStatus_ &= 0x1f;
        }
    }

    if (scanlineCycle_ == -1)
    {
        scanlineCycle_++;
        return;
    }


    if (currentScanLine_ < 240)
    {
        if (scanlineCycle_ < 256)
        {
            if (currentScanLine_ >= 0)
            {
                pixelRendered_ = false;
                currentPixel_ = palette_[0];

                if ((ppuMask_ & 0x08) != 0)
                {
                    BackgroundRender();
                }

                if ((ppuMask_ & 0x10) != 0)
                {
                    SpriteRender();
                    SpriteTick();
                }

                display_.WritePixel(currentPixel_);
            }

            if ((ppuMask_ & 0x18) != 0)
            {
                BackgroundLoadTick();
                BackgroundTick();
                SpriteEvaluationTick();
            }
        }
        else if (scanlineCycle_ == 256)
        {
            scanlineSpriteCount_ = oamCopyIndex_ >> 2;
            sprite0Visible_ = sprite0Selected_;
            sprite0Selected_ = false;
            oamAddress_ = 0;
            oamCopyIndex_ = 0;
            spriteIndex_ = 0;

            display_.HBlank();

            if ((ppuMask_ & 0x18) != 0)
            {
                BackgroundHReset();
            }
        }
        else if (scanlineCycle_ >= 256 && scanlineCycle_ < 320)
        {
            if ((ppuMask_ & 0x18) != 0)
            {
                // sprite tile loading
                SpriteLoadTick();
            }

            if (currentScanLine_ < 0 && (scanlineCycle_ >= 279 && scanlineCycle_ < 304))
            {
                if ((ppuMask_ & 0x18) != 0)
                {
                    BackgroundVReset();
                }
            }
        }
        else if (scanlineCycle_ >= 320 && scanlineCycle_ < 336)
        {
            if ((ppuMask_ & 0x18) != 0)
            {
                BackgroundLoadTick();
                BackgroundTick();
            }
        }
    }

    scanlineCycle_++;
    if (scanlineCycle_ == 340)
    {
        scanlineCycle_ = -1;

        currentScanLine_++;

        if (currentScanLine_ == 261)
        {
            currentScanLine_ = -1;
        }
    }
}

uint8_t Ppu::Read(uint16_t address)
{
    address &= 0x07;

    if (address == 0x02)
    {
        auto status = ppuStatus_;

        ppuStatus_ &= 0x7f;
        addressLatch_ = false;

        return status;
    }
    else if (address == 0x07)
    {
        auto data = ppuData_;
        auto ppuAddress = (uint16_t)(currentAddress_ & 0x3fff);
        ppuData_ = bus_.PpuRead(ppuAddress);

        if (ppuAddress >= 0x3f00)
        {
            data = palette_[ppuAddress & 0x1f];
        }

        if ((ppuControl_ & 0x04) != 0)
        {
            currentAddress_ += 32;
        }
        else
        {
            currentAddress_ += 1;
        }

        return data;
    }

    return 0;
}

void Ppu::Write(uint16_t address, uint8_t value)
{
    address &= 0x07;
    switch (address)
    {
    case 0:
        ppuControl_ = value;

        backgroundPatternBase_ = (uint16_t)((ppuControl_ & 0x10) << 8);
        spritePatternBase_ = (uint16_t)((ppuControl_ & 0x08) << 9);

        initialAddress_ &= 0xf3ff;
        initialAddress_ |= (uint16_t)((value & 3) << 10);
        return;

    case 1:
        ppuMask_ = value;
        return;

    case 3:
        oamAddress_ = value;
        return;

    case 5:
        if (!addressLatch_)
        {
            initialAddress_ &= 0xffe0;
            initialAddress_ |= (uint8_t)(value >> 3);
            SetFineX((uint8_t)(value & 7));
            addressLatch_ = true;
        }
        else
        {
            initialAddress_ &= 0x8c1f;
            initialAddress_ |= (uint16_t)((value & 0xf8) << 2);
            initialAddress_ |= (uint16_t)((value & 0x07) << 12);
            addressLatch_ = false;
        }
        return;

    case 6:
        if (!addressLatch_)
        {
            initialAddress_ = (uint16_t)(((value & 0x3f) << 8) | (initialAddress_ & 0xff));
            addressLatch_ = true;
        }
        else
        {
            initialAddress_ = (uint16_t)((initialAddress_ & 0xff00) | value);
            currentAddress_ = initialAddress_;
            addressLatch_ = false;
        }
        return;

    case 7:
    {
        auto writeAddress = (uint16_t)(currentAddress_ & 0x3fff);
        if (writeAddress >= 0x3f00)
        {
            if ((writeAddress & 0x03) == 0)
            {
                // zero colors are mirrored
                palette_[writeAddress & 0x000f] = value;
                palette_[(writeAddress & 0x000f) | 0x0010] = value;
            }
            else
            {
                palette_[writeAddress & 0x001f] = value;
            }
        }
        else
        {
            bus_.PpuWrite(writeAddress, value);
        }

        if ((ppuControl_ & 0x04) != 0)
        {
            currentAddress_ += 32;
        }
        else
        {
            currentAddress_ += 1;
        }
        return;
    }

    default:
        return;
    }
}

void Ppu::DmaWrite(uint8_t value)
{
    oam_[oamAddress_++] = value;
}

void Ppu::SetFineX(uint8_t value)
{
    patternBitShift_ = 15 - value;
    patternMask_ = 1 << patternBitShift_;

    // 2 off since this maps to bits 2 and 3
    attributeBitShift_ = 28 - 2 * value;
    attributeMask_ = 3 << (attributeBitShift_ + 2);
}

void Ppu::BackgroundRender()
{
    auto index = (uint8_t)((patternShiftHigh_ & patternMask_) >> (patternBitShift_ - 1)
        | (patternShiftLow_ & patternMask_) >> patternBitShift_);

    if (index != 0)
    {
        pixelRendered_ = true;

        index |= (uint8_t)((attributeShift_ & attributeMask_) >> attributeBitShift_); // palette
        currentPixel_ = palette_[index];
    }
}

void Ppu::BackgroundLoadTick()
{
    switch (scanlineCycle_ & 0x07)
    {
    case 0:

        patternShiftHigh_ |= nextPatternByteHigh_;
        patternShiftLow_ |= nextPatternByteLow_;
        attributeShift_ |= nextAttributeShift_;
        break;

    case 1:
    {
        auto tileAddress = (uint16_t)(0x2000 | currentAddress_ & 0x0fff);
        nextTileId_ = bus_.PpuRead(tileAddress);
        break;
    }

    case 3:
    {
        auto attributeAddress = (uint16_t)(
            0x2000 | (currentAddress_ & 0x0C00) | // select table
            0x03C0 | // attribute block at end of table
            (currentAddress_ >> 4) & 0x0038 | // 3 bits of tile y
            (currentAddress_ >> 2) & 0x0007); // 3 bits of tile x

        auto attributes = bus_.PpuRead(attributeAddress);

        // use one more bit of the tile x and y to get the quadrant
        attributes >>= ((currentAddress_ & 0x0040) >> 4) | (currentAddress_ & 0x0002);
        attributes &= 0x03;

        // we've determined the palette index.  Lets duplicate this 8 times for the tile pixels
        nextAttributeShift_ = attributes;
        nextAttributeShift_ |= (uint16_t)(nextAttributeShift_ << 2);
        nextAttributeShift_ |= (uint16_t)(nextAttributeShift_ << 4);
        nextAttributeShift_ |= (uint16_t)(nextAttributeShift_ << 8);
        break;
    }

    case 5:
        // address is 000PTTTTTTTT0YYY
        patternAddress_ = (uint16_t)
            (backgroundPatternBase_ | // pattern selector
            (nextTileId_ << 4) |
            (currentAddress_ >> 12)); // fineY

        nextPatternByteLow_ = bus_.PpuRead(patternAddress_);
        break;

    case 7:
        // address is 000PTTTTTTTT1YYY
        nextPatternByteHigh_ = bus_.PpuRead((uint16_t)(patternAddress_ | 8));

        if (scanlineCycle_ == 255)
        {
            // adjust y scroll
            currentAddress_ += 0x1000;
            currentAddress_ &= 0x7fff;

            if ((currentAddress_ & 0x7000) == 0)
            {
                // move to the next row
                if ((currentAddress_ & 0x03e0) == 0x03e0)
                {
                    currentAddress_ &= 0x7c1f;
                    currentAddress_ ^= 0x0800;
                }
                else
                {
                    currentAddress_ += 0x0020;
                }
            }
        }
        else
        {
            // increment the x part of the address
            if ((currentAddress_ & 0x001f) == 0x001f)
            {
                currentAddress_ &= 0xffe0;
                currentAddress_ ^= 0x0400;
            }
            else
            {
                currentAddress_++;
            }
        }
        break;
    }
}

void Ppu::BackgroundTick()
{
    patternShiftHigh_ <<= 1;
    patternShiftLow_ <<= 1;
    attributeShift_ <<= 2;
}

void Ppu::BackgroundVReset()
{
    currentAddress_ &= 0x041f;
    currentAddress_ |= (uint16_t)(initialAddress_ & 0xfbe0);
}

void Ppu::BackgroundHReset()
{
    currentAddress_ &= 0xfbe0;
    currentAddress_ |= (uint16_t)(initialAddress_ & 0x041f);
}

void Ppu::SpriteRender()
{
    bool drawnSprite = false;

    for (auto i = 0; i < scanlineSpriteCount_; i++)
    {
        if (sprites_[i].X != 0)
        {
            continue;
        }

        uint8_t index;
        if ((sprites_[i].attributes & 0x40) == 0)
        {
            index = (uint8_t)((sprites_[i].patternShiftHigh & 0x80) >> 6 | (sprites_[i].patternShiftLow & 0x80) >> 7);

            sprites_[i].patternShiftHigh <<= 1;
            sprites_[i].patternShiftLow <<= 1;
        }
        else
        {
            index = (uint8_t)((sprites_[i].patternShiftHigh & 0x01) << 1 | sprites_[i].patternShiftLow & 0x01);

            sprites_[i].patternShiftHigh >>= 1;
            sprites_[i].patternShiftLow >>= 1;
        }

        if (index != 0 && !drawnSprite)
        {
            drawnSprite = true;

            if (pixelRendered_)
            {
                if (i == 0 && sprite0Visible_)
                {
                    ppuStatus_ |= 0x40;
                }

                if ((sprites_[i].attributes & 0x20) != 0)
                {
                    continue;
                }
            }

            index |= (uint8_t)((0x04 | (sprites_[i].attributes & 0x03)) << 2); // palette
            currentPixel_ = palette_[index];
        }
    }
}

void Ppu::SpriteTick()
{
    for (auto i = 0; i < scanlineSpriteCount_; i++)
    {
        if (sprites_[i].X > 0)
        {
            sprites_[i].X--;
        }
    }
}

void Ppu::SpriteEvaluationTick()
{
    if ((scanlineCycle_ & 1) == 0)
    {
        // setting up the read/write
        return;
    }

    if (scanlineCycle_ < 64)
    {
        oamCopy_[scanlineCycle_ / 2] = oamData_ = 0xff;
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

    auto spriteRow = (uint32_t)(currentScanLine_ - oamData_);
    bool visible = spriteRow < 8;

    if (oamAddress_ == 0)
    {
        sprite0Selected_ = visible;
    }

    if (visible)
    {
        if (oamCopyIndex_ >= 32)
        {
            ppuStatus_ |= 0x20;
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

void Ppu::SpriteLoadTick()
{
    if (spriteIndex_ >= scanlineSpriteCount_)
    {
        return;
    }

    switch (scanlineCycle_ & 0x07)
    {
    case 5:
    {
        auto oamAddress = spriteIndex_ << 2;

        auto attributes = oamCopy_[oamAddress + 2];
        sprites_[spriteIndex_].attributes = attributes;
        sprites_[spriteIndex_].X = oamCopy_[oamAddress + 3];

        auto tileId = oamCopy_[oamAddress + 1];

        auto tileFineY = currentScanLine_ - oamCopy_[oamAddress];

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
