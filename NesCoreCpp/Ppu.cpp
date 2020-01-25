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

    if (updateBaseAddress_)
    {
        Sync(targetCycle_);
        background_.CurrentAddress = initialAddress_;
        updateBaseAddress_ = false;
    }

    if (targetCycle_ >= 340)
    {
        Sync(340);

        // the target cycle starts at -1 which gives us our extra tick at the start of the line
        targetCycle_ -= 341;
    }

    // always sync on the post render scanline to ensure the NMI is triggered correctly.
    else if (enableVBlankInterrupt_ && currentScanline_ == 241 && !inVBlank_)
    {
        PostRenderScanline(targetCycle_);
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
        // these settings take one cycle to take effect so we'll run ahead slightly
        Sync(targetCycle_ + 1);
        background_.Enable(value & 0x08);
        enableForeground_ = (value & 0x10) != 0;
        enableRendering_ = (value & 0x18) != 0;
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
            // This write takes a couple of cycles to take effect
            Sync(targetCycle_);

            initialAddress_ = (uint16_t)((initialAddress_ & 0xff00) | value);
            // update the address in 3 cycles time.
            updateBaseAddress_ = true;
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

void Ppu::Sync(int32_t targetCycle)
{
    if (targetCycle <= scanlineCycle_)
        return;

    if (currentScanline_ < 240)
    {
        RenderScanline(targetCycle);
    }
    else if (currentScanline_ == 241)
    {
        PostRenderScanline(targetCycle);
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
        display_.HBlank();

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

            background_.RunRender(scanlineCycle_, maxIndex);

            if (enableForeground_)
            {
                sprites_.RunRender(scanlineCycle_, maxIndex, background_.ScanlinePixels());
            }

            sprites_.RunEvaluation(currentScanline_, scanlineCycle_, maxIndex);
        }

        scanlineCycle_ = maxIndex;
        if (scanlineCycle_ >= targetCycle)
            return;
    }

    if (scanlineCycle_ == 256)
    {
        // merge the sprites and the background
        auto& backgroundPixels = background_.ScanlinePixels();
        auto& spriteAttributes = sprites_.ScanlineAttributes();
        auto& spritePixels = sprites_.ScanlinePixels();

        uint8_t pixel;
        for (auto i = 0; i < 256; i++)
        {
            if (spriteAttributes[i] & 0x20)
                pixel = backgroundPixels[i] ? backgroundPixels[i] : spritePixels[i];
            else
                pixel = spritePixels[i] ? spritePixels[i] : backgroundPixels[i];

            display_.WritePixel(rgbPalette_[pixel]);
        }

        sprites_.HReset();
        display_.HBlank();

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

void Ppu::PostRenderScanline(int32_t targetCycle)
{
    if (scanlineCycle_ == 0)
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

    scanlineCycle_ = targetCycle;
}
