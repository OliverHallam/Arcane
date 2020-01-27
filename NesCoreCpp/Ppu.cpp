#include "Ppu.h"

#include "Bus.h"
#include "Display.h"

Ppu::Ppu(Bus& bus, Display& display) :
    bus_{ bus },
    display_{ display },
    background_{ bus },
    sprites_{ bus }
{
}

uint32_t Ppu::FrameCount()
{
    return frameCount_;
}

void Ppu::Tick3()
{
    targetCycle_ += 3;

    if (hasDeferredUpdate_)
    {
        RunDeferredUpdate();
    }

    if (targetCycle_ >= 340)
    {
        SyncScanline();
    }
}

uint8_t Ppu::Read(uint16_t address)
{
    address &= 0x07;

    if (address == 0x02)
    {
        addressLatch_ = false;

        uint8_t status = 0;

        if (inVBlank_)
        {
            status |= 0x80;
            inVBlank_ = false;
        }

        if (sprites_.Sprite0Hit())
            status |= 0x40;
        else if (sprites_.Sprite0Visible())
        {
            Sync(targetCycle_);

            if (sprites_.Sprite0Hit())
            {
                status |= 0x40;
            }
        }

        if (sprites_.SpriteOverflow())
            status |= 0x20;

        return status;
    }
    else if (address == 0x07)
    {
        Sync(targetCycle_);
        auto data = ppuData_;

        // the address is aliased with the PPU background render address, so we use that.
        auto ppuAddress = (uint16_t)(background_.CurrentAddress & 0x3fff);
        ppuData_ = bus_.PpuRead(ppuAddress);

        if (ppuAddress >= 0x3f00)
        {
            data = palette_[ppuAddress & 0x1f];
        }

        background_.CurrentAddress += addressIncrement_;
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
        Sync(targetCycle_);
        // PPUCTRL flags
        enableVBlankInterrupt_ = (value & 0x80) != 0;
        // TODO: Sprite size (value & 0x20)
        background_.SetBasePatternAddress((uint16_t)((value & 0x10) << 8));
        sprites_   .SetBasePatternAddress((uint16_t)((value & 0x08) << 9));
        addressIncrement_ = (value & 0x04) != 0 ? 32 : 1;

        // set base nametable address
        initialAddress_ &= 0xf3ff;
        initialAddress_ |= (uint16_t)((value & 3) << 10);
        return;

    case 1:
        mask_ = value;
        updateMask_ = true;
        hasDeferredUpdate_ = true;
        return;

    case 3:
        sprites_.SetOamAddress(value);
        return;

    case 5:
        if (targetCycle_ > 256)
        {
            // we only need to sync if we are pending an HReset
            Sync(targetCycle_);
        }

        if (!addressLatch_)
        {
            initialAddress_ &= 0xffe0;
            initialAddress_ |= (uint8_t)(value >> 3);
            background_.SetFineX((uint8_t)(value & 7));
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
            if (targetCycle_ > 256)
            {
                // we only need to sync if we are pending an HReset
                Sync(targetCycle_);
            }

            initialAddress_ = (uint16_t)(((value & 0x3f) << 8) | (initialAddress_ & 0xff));
            addressLatch_ = true;
        }
        else
        {
            initialAddress_ = (uint16_t)((initialAddress_ & 0xff00) | value);
            // update the address in 3 cycles time.
            updateBaseAddress_ = true;
            hasDeferredUpdate_ = true;
            addressLatch_ = false;
        }
        return;

    case 7:
    {
        Sync(targetCycle_);

        auto writeAddress = (uint16_t)(background_.CurrentAddress & 0x3fff);
        if (writeAddress >= 0x3f00)
        {
            if ((writeAddress & 0x03) == 0)
            {
                // zero colors are mirrored
                palette_[writeAddress & 0x000f] = value;
                palette_[(writeAddress & 0x000f) | 0x0010] = value;

                auto rgb = display_.GetPixel(value);
                rgbPalette_[writeAddress & 0x000f] = rgb;
                rgbPalette_[(writeAddress & 0x000f) | 0x0010] = rgb;
            }
            else
            {
                palette_[writeAddress & 0x001f] = value;
                rgbPalette_[writeAddress & 0x001f] = display_.GetPixel(value);
            }
        }
        else
        {
            bus_.PpuWrite(writeAddress, value);
        }

        background_.CurrentAddress += addressIncrement_;
        return;
    }

    default:
        return;
    }
}

void Ppu::DmaWrite(uint8_t value)
{
    sprites_.WriteOam(value);
}

void Ppu::RunDeferredUpdate()
{
    if (enterVBlank_)
    {
        EnterVBlank();
        enterVBlank_ = false;
    }

    if (updateBaseAddress_)
    {
        Sync(targetCycle_);
        background_.CurrentAddress = initialAddress_;
        updateBaseAddress_ = false;
    }
    else if (updateMask_)
    {
        // mask updates after 2 cycles
        Sync(targetCycle_ - 1);

        enableBackground_ = (mask_ & 0x08) != 0;
        enableForeground_ = (mask_ & 0x10) != 0;
        enableRendering_ = (mask_ & 0x18) != 0;
        updateMask_ = false;
    }

    hasDeferredUpdate_ = false;
}

void Ppu::SyncScanline()
{
    // we're at the end of the scanline, so lets render the whole thing
    if (currentScanline_ < 240)
    {
        if (scanlineCycle_ == 0)
        {
            RenderScanline();
        }
        else
        {
            RenderScanline(340);
        }
    }
    else if (currentScanline_ == 240)
    {
        scanlineCycle_ = 0;
        currentScanline_ = 241;
        targetCycle_ -= 341;

        // if we've stepped over the start of VBlank, we should sync that too.
        if (targetCycle_ > 0)
        {
            EnterVBlank();
        }
        else
        {
            // otherwise we enter VBlank on the next update
            enterVBlank_ = true;
            hasDeferredUpdate_ = true;
        }
        return;
    }
    else if (currentScanline_ == 261)
    {
        PreRenderScanline(340);
        scanlineCycle_ = 0;
        currentScanline_ = 0;
        targetCycle_ -= 341;
        return;
    }

    scanlineCycle_ = 0;
    currentScanline_++;

    // the target cycle starts at -1 which gives us our extra tick at the start of the line
    targetCycle_ -= 341;
}

void Ppu::Sync(int32_t targetCycle)
{
    if (targetCycle <= scanlineCycle_)
        return;

    if (currentScanline_ < 240)
    {
        RenderScanline(targetCycle);
    }
    else if (currentScanline_ == 261)
    {
        PreRenderScanline(targetCycle);

        if (scanlineCycle_ >= 340)
        {
            scanlineCycle_ = 0;
            currentScanline_ = 0;
        }
        return;
    }
    else
    {
        scanlineCycle_ = targetCycle;
    }

    if (scanlineCycle_ >= 340)
    {
        scanlineCycle_ = 0;
        currentScanline_++;
    }
}

void Ppu::PreRenderScanline(int32_t targetCycle)
{
    if (scanlineCycle_ == 0)
    {
        sprites_.VReset();
        inVBlank_ = false;
    }

    if (scanlineCycle_ < 256)
    {
        auto maxIndex = std::min(256, targetCycle);

        if (enableRendering_)
        {
            background_.RunLoad(scanlineCycle_, maxIndex);

            sprites_.RunEvaluation(currentScanline_, scanlineCycle_, maxIndex);
        }

        scanlineCycle_ = maxIndex;
        if (scanlineCycle_ == targetCycle)
            return;
    }

    if (scanlineCycle_ == 256)
    {
        sprites_.HReset();

        if (enableRendering_)
        {
            background_.HReset(initialAddress_);
        }

        scanlineCycle_ = 257;
        if (scanlineCycle_ == targetCycle)
            return;
    }

    if (scanlineCycle_ < 320)
    {
        if (enableRendering_)
        {
            if (targetCycle >= 280 && scanlineCycle_ < 304)
            {
                background_.VReset(initialAddress_);
            }

            auto maxCycle = std::min(targetCycle, 320);

            // sprite tile loading
            sprites_.RunLoad(-1, scanlineCycle_, maxCycle);

            scanlineCycle_ = maxCycle;
        }
        else
        {
            scanlineCycle_ = 320;
        }

        if (scanlineCycle_ >= targetCycle)
            return;
    }

    if (enableRendering_)
    {
        auto maxCycle = std::min(targetCycle, 336);

        // sprite tile loading
        background_.RunLoad(scanlineCycle_, maxCycle);
    }

    scanlineCycle_ = targetCycle;
}

void Ppu::RenderScanline(int32_t targetCycle)
{
    if (scanlineCycle_ == 0)
    {
        background_.BeginScanline();
    }

    if (scanlineCycle_ < 256)
    {
        auto maxIndex = std::min(256, targetCycle);

        if (enableRendering_)
        {
            background_.RunLoad(scanlineCycle_, maxIndex);

            if (enableBackground_)
            {
                background_.RunRender(scanlineCycle_, maxIndex);
            }
            else
            {
                background_.RunRenderDisabled(scanlineCycle_, maxIndex);
            }

            if (enableForeground_)
            {
                sprites_.RunRender(scanlineCycle_, maxIndex, background_.ScanlinePixels());
            }

            sprites_.RunEvaluation(currentScanline_, scanlineCycle_, maxIndex);
        }
        else
        {
            background_.RunRenderDisabled(scanlineCycle_, maxIndex);
        }

        scanlineCycle_ = maxIndex;
        if (scanlineCycle_ >= targetCycle)
            return;
    }

    if (scanlineCycle_ == 256)
    {
        FinishRender();

        scanlineCycle_ = 257;
        if (scanlineCycle_ == targetCycle)
            return;
    }

    if (scanlineCycle_ < 320)
    {
        auto maxIndex = std::min(320, targetCycle);

        if (enableRendering_)
        {
            sprites_.RunLoad(currentScanline_, scanlineCycle_, maxIndex);
        }

        scanlineCycle_ = maxIndex;
        if (scanlineCycle_ == targetCycle)
            return;
    }

    if (scanlineCycle_ < 336)
    {
        auto maxIndex = std::min(336, targetCycle);
        if (enableRendering_)
        {
            background_.RunLoad(scanlineCycle_, maxIndex);
        }
    }

    scanlineCycle_ = targetCycle;
}

void Ppu::RenderScanline()
{
    background_.BeginScanline();

    // TODO: we can optimize the disabled case here
    if (enableRendering_)
    {
        background_.RunLoad();

        if (enableBackground_)
        {
            background_.RunRender(0, 256);
        }
        else
        {
            background_.RunRenderDisabled(0, 256);
        }

        if (enableForeground_)
        {
            sprites_.RunRender(0, 256, background_.ScanlinePixels());
        }

        sprites_.RunEvaluation(currentScanline_, 0, 256);
    }
    else
    {
        background_.RunRenderDisabled(0, 256);
    }

    FinishRender();

    if (enableRendering_)
    {
        sprites_.RunLoad(currentScanline_);
        background_.RunLoad(320, 336);
    }
}

void Ppu::FinishRender()
{
    // merge the sprites and the background
    auto& backgroundPixels = background_.ScanlinePixels();
    auto& spriteAttributes = sprites_.ScanlineAttributes();
    auto& spritePixels = sprites_.ScanlinePixels();

    auto scanline = display_.GetScanlinePtr();
    if (sprites_.SpritesVisible())
    {
        for (auto i = 0; i < 256; i++)
        {
            auto pixel = backgroundPixels[i];
            auto spritePixel = spritePixels[i];
            if (spritePixel)
            {
                if (spriteAttributes[i] & 0x20)
                {
                    if (!pixel)
                        pixel = spritePixel;
                }
                else
                {
                    pixel = spritePixel;
                }
            }

            scanline[i] = rgbPalette_[pixel];
        }
    }
    else
    {
        for (auto i = 0; i < 256; i++)
        {
            auto pixel = backgroundPixels[i];
            scanline[i] = rgbPalette_[pixel];
        }
    }

    sprites_.HReset();
    display_.HBlank();

    if (enableRendering_)
    {
        background_.HReset(initialAddress_);
    }
}

void Ppu::EnterVBlank()
{
    display_.VBlank();

    frameCount_++;

    if (enableVBlankInterrupt_)
    {
        bus_.SignalNmi();
    }

    // The VBlank flag of the PPU is set at tick 1 (the second tick) of scanline 241
    inVBlank_ = true;
}
