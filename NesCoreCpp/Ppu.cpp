#include "Ppu.h"

#include "Bus.h"
#include "Display.h"

#include <cassert>

Ppu::Ppu(Bus& bus, Display& display) :
    bus_{ bus },
    display_{ display },
    background_{ bus },
    sprites_{ bus }
{
    bus_.Schedule(342 / 3, SyncEvent::PpuScanline);
}

uint32_t Ppu::FrameCount()
{
    return frameCount_;
}

void Ppu::Tick3()
{
    targetCycle_ += 3;

    if (bus_.SensitiveToChrA12())
    {
        SyncA12();
    }
}

void Ppu::Sync()
{
    Sync(targetCycle_);
}

void Ppu::SyncA12()
{
    if (enableRendering_)
    {
        if (currentScanline_ < 240)
        {
            if (targetCycle_ >= 256 && scanlineCycle_ <= 320)
            {
                Sync(targetCycle_);
            }
        }
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

        // if the status is read at the point we were about to signal a vblank, that can cause it to be skipped.
        signalVBlank_ = false;

        // the flag should be reset at the start of the pre-render row, we need to sync in that case
        if (sprites_.Sprite0Hit())
        {
            // the pre-render line will reset this but we haven't synced yet
            if (currentScanline_ != 261)
                status |= 0x40;
        }
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
            data = palette_[ppuAddress & 0x1f] & grayscaleMask_;
        }

        background_.CurrentAddress += addressIncrement_;
        background_.CurrentAddress &= 0x7fff;
        bus_.SetChrA12(background_.CurrentAddress & 0x1000);
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
    {
        Sync(targetCycle_);

        auto wasVblankInterruptEnabled = enableVBlankInterrupt_;

        // PPUCTRL flags
        enableVBlankInterrupt_ = (value & 0x80) != 0;
        sprites_.SetLargeSprites((value & 0x20) != 0);
        background_.SetBasePatternAddress((uint16_t)((value & 0x10) << 8));
        sprites_.SetBasePatternAddress((uint16_t)((value & 0x08) << 9));
        addressIncrement_ = (value & 0x04) != 0 ? 32 : 1;

        // set base nametable address
        initialAddress_ &= 0xf3ff;
        initialAddress_ |= (uint16_t)((value & 3) << 10);

        if (enableVBlankInterrupt_ && !wasVblankInterruptEnabled && inVBlank_)
        {
            signalVBlank_ = true;
            bus_.Schedule(1, SyncEvent::PpuStateUpdate);
        }

        return;
    }

    case 1:
        mask_ = value;
        updateMask_ = true;
        bus_.Schedule(1, SyncEvent::PpuStateUpdate);
        return;

    case 3:
        sprites_.SetOamAddress(value);
        return;

    case 5:
        if (!addressLatch_)
        {
            // The fine X takes effect immediately, so we need to sync
            Sync(targetCycle_);

            initialAddress_ &= 0xffe0;
            initialAddress_ |= (uint8_t)(value >> 3);
            background_.SetFineX((uint8_t)(value & 7));
            addressLatch_ = true;
        }
        else
        {
            if (targetCycle_ > 256)
            {
                // we only need to sync if we are pending an HReset
                Sync(targetCycle_);
            }

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
            bus_.Schedule(1, SyncEvent::PpuStateUpdate);

            addressLatch_ = false;
        }
        return;

    case 7:
    {
        Sync(targetCycle_);

        auto writeAddress = (uint16_t)(background_.CurrentAddress & 0x3fff);
        if (writeAddress >= 0x3f00)
        {
            SyncComposite(targetCycle_);

            value &= 0x3f;

            if ((writeAddress & 0x03) == 0)
            {
                writeAddress &= 0x000f;

                // zero colors are mirrored
                palette_[writeAddress] = value;
                palette_[writeAddress | 0x0010] = value;

                auto rgb = display_.GetPixel(value & grayscaleMask_, emphasis_);
                rgbPalette_[writeAddress] = rgb;
                rgbPalette_[writeAddress | 0x0010] = rgb;
            }
            else
            {
                writeAddress &= 0x0001f;

                palette_[writeAddress] = value;
                rgbPalette_[writeAddress] = display_.GetPixel(value & grayscaleMask_, emphasis_);
            }
        }
        else
        {
            bus_.PpuWrite(writeAddress, value);
        }

        background_.CurrentAddress += addressIncrement_;
        background_.CurrentAddress &= 0x7fff;
        bus_.SetChrA12(background_.CurrentAddress & 0x1000);
        // TODO: when rendering is enabled, this behaves weirdly.
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

void Ppu::SyncState()
{
    if (signalVBlank_)
    {
        SignalVBlank();
        signalVBlank_ = false;
    }

    if (updateBaseAddress_)
    {
        Sync(targetCycle_);
        bus_.SetChrA12((initialAddress_ & 0x1000) != 0);
        background_.CurrentAddress = initialAddress_;
        updateBaseAddress_ = false;
    }
    else if (updateMask_)
    {
        // mask updates after 2 cycles
        Sync(targetCycle_ - 1);

        background_.EnableLeftColumn((mask_ & 0x02) != 0);
        sprites_.EnableLeftColumn((mask_ & 0x04) != 0);
        enableBackground_ = (mask_ & 0x08) != 0;
        enableForeground_ = (mask_ & 0x10) != 0;

        auto prevRenderingEnabled = enableRendering_;
        enableRendering_ = (mask_ & 0x18) != 0;

        if (enableRendering_ && !prevRenderingEnabled && (currentScanline_ < 240))
        {
            background_.EnableRendering(targetCycle_ - 1);
        }

        grayscaleMask_ = (mask_ & 0x01) ? 0x30 : 0xff;
        auto newEmphasis = (mask_ & 0xe0) >> 5;
        if (newEmphasis != emphasis_)
        {
            SyncComposite(targetCycle_ - 1);
            emphasis_ = newEmphasis;

            // rebuild palette
            for (auto index = 0; index < 32; index++)
            {
                rgbPalette_[index] = display_.GetPixel(palette_[index] & grayscaleMask_, emphasis_);
            }
        }

        updateMask_ = false;
    }
}

void Ppu::SyncScanline()
{
    if (targetCycle_ < 340)
        return;

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

        currentScanline_++;
    }
    else if (currentScanline_ == 240)
    {
        EnterVBlank();

        // if we've stepped over the start of VBlank, we should sync that too.
        if (targetCycle_ > 0)
        {
            SignalVBlank();
        }
        else
        {
            // otherwise we process the VBlank on the next update
            signalVBlank_ = true;
            bus_.Schedule(1, SyncEvent::PpuStateUpdate);
        }

        currentScanline_ = 241;
    }
    else if (currentScanline_ == 261)
    {
        PreRenderScanline(340);
        currentScanline_ = 0;
    }
    else
    {
        currentScanline_++;
    }

    // the target cycle starts at -1 which gives us our extra tick at the start of the line
    scanlineCycle_ = 0;
    targetCycle_ -= 341;
    bus_.Schedule((342 - targetCycle_) / 3, SyncEvent::PpuScanline);
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
        return;
    }
    else
    {
        scanlineCycle_ = targetCycle;
    }
}

void Ppu::SyncComposite(int32_t targetCycle)
{
    if (currentScanline_ >= 240)
        return;
    if (targetCycle < 0 || targetCycle > 256)
        return;

    Composite(compositeCycle_, targetCycle);
    compositeCycle_ = targetCycle;
}

void Ppu::PreRenderScanline(int32_t targetCycle)
{
    if (scanlineCycle_ == 0)
    {
        background_.BeginScanline();
        sprites_.VReset();
        inVBlank_ = false;
    }

    if (scanlineCycle_ < 256)
    {
        auto maxIndex = std::min(256, targetCycle);

        if (enableRendering_)
        {
            background_.RunLoad(scanlineCycle_, maxIndex);
        }

        scanlineCycle_ = maxIndex;
        if (scanlineCycle_ == targetCycle)
            return;
    }

    if (scanlineCycle_ == 256)
    {
        if (enableRendering_)
            background_.HReset(initialAddress_);
        else
            background_.HResetRenderDisabled();

        scanlineCycle_ = 257;
        if (scanlineCycle_ == targetCycle)
            return;
    }

    if (scanlineCycle_ < 320)
    {
        auto maxCycle = std::min(targetCycle, 320);

        sprites_.DummyLoad();

        if (enableRendering_)
        {
            if (targetCycle >= 280 && scanlineCycle_ < 304)
            {
                background_.VReset(initialAddress_);
            }

            scanlineCycle_ = maxCycle;
        }
        else
        {
            scanlineCycle_ = maxCycle;
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

            if (enableForeground_ && currentScanline_ != 0)
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
            background_.RenderScanline();
        }
        else
        {
            background_.RunRenderDisabled(0, 256);
        }

        if (enableForeground_ && currentScanline_ != 0)
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

void Ppu::Composite(int32_t startCycle, int32_t endCycle)
{
    // merge the sprites and the background
    auto& backgroundPixels = background_.ScanlinePixels();
    auto& spriteAttributes = sprites_.ScanlineAttributes();
    auto& spritePixels = sprites_.ScanlinePixels();

    auto scanline = display_.GetScanlinePtr();
    if (sprites_.SpritesVisible())
    {
        for (auto i = startCycle; i < endCycle; i++)
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
        for (auto i = startCycle; i < endCycle; i++)
        {
            auto pixel = backgroundPixels[i];
            scanline[i] = rgbPalette_[pixel];
        }
    }
}

void Ppu::FinishRender()
{
    Composite(compositeCycle_, 256);
    compositeCycle_ = 0;

    sprites_.HReset();
    display_.HBlank();

    if (enableRendering_)
        background_.HReset(initialAddress_);
    else
        background_.HResetRenderDisabled();
}

void Ppu::EnterVBlank()
{
    display_.VBlank();

    bus_.OnFrame();
    frameCount_++;
}

void Ppu::SignalVBlank()
{
    // The VBlank flag of the PPU is set at tick 1 (the second tick) of scanline 241
    inVBlank_ = true;
    if (enableVBlankInterrupt_)
        bus_.SignalNmi();
}
