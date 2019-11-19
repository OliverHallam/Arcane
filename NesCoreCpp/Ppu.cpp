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
    // TODO: we need to sync the trigger for an NMI too

    targetCycle_ += 3;
    if (targetCycle_ >= 340)
    {
        Sync(340);
        targetCycle_ -= 341;
    }
}

uint8_t Ppu::Read(uint16_t address)
{
    Sync(targetCycle_);

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

        if (sprites_.SpriteOverflow())
            status |= 0x20;

        return status;
    }
    else if (address == 0x07)
    {
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
    Sync(targetCycle_);

    address &= 0x07;
    switch (address)
    {
    case 0:
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
        enableBackground_ = (value & 0x08) != 0;
        enableForeground_ = (value & 0x10) != 0;
        enableRendering_ = (value & 0x18) != 0;
        return;

    case 3:
        sprites_.SetOamAddress(value);
        return;

    case 5:
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
            initialAddress_ = (uint16_t)(((value & 0x3f) << 8) | (initialAddress_ & 0xff));
            addressLatch_ = true;
        }
        else
        {
            initialAddress_ = (uint16_t)((initialAddress_ & 0xff00) | value);
            background_.CurrentAddress = initialAddress_;
            addressLatch_ = false;
        }
        return;

    case 7:
    {
        auto writeAddress = (uint16_t)(background_.CurrentAddress & 0x3fff);
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
    if (targetCycle_ <= currentScanline_)
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
            // the target cycle starts at -1 which gives us our extra tick at the start of the line
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
        // the target cycle starts at -1 which gives us our extra tick at the start of the line
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

    if (!enableRendering_)
    {
        if (targetCycle <= 256)
        {
            scanlineCycle_ = targetCycle;
            return;
        }

        scanlineCycle_ = 256;
    }
    else
    {
        if (scanlineCycle_ < 256)
        {
            auto maxIndex = std::min(256, targetCycle);

            for (auto index = scanlineCycle_; index < maxIndex; index++)
            {
                background_.Load(index);
            }

            for (auto index = scanlineCycle_; index < maxIndex; index++)
            {
                background_.Tick(index);
                sprites_.EvaluationTick(currentScanline_, index);

            }

            scanlineCycle_ = maxIndex;
            if (scanlineCycle_ == targetCycle)
                return;
        }
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

    while (scanlineCycle_ < 320)
    {
        if (enableRendering_)
        {
            // sprite tile loading
            sprites_.LoadTick(-1, scanlineCycle_);

            if (scanlineCycle_ >= 279 && scanlineCycle_ < 304)
            {
                background_.VReset(initialAddress_);
            }
        }

        scanlineCycle_++;
        if (scanlineCycle_ == targetCycle)
            return;
    }

    while (scanlineCycle_ < 336)
    {
        if (enableRendering_)
        {
            background_.Load(scanlineCycle_);
            background_.Tick(scanlineCycle_);
        }

        scanlineCycle_++;
        if (scanlineCycle_ == targetCycle)
            return;
    }

    scanlineCycle_ = targetCycle;
}

void Ppu::RenderScanline(int32_t targetCycle)
{
    if (enableRendering_ && scanlineCycle_ < 256)
    {
        auto maxIndex = std::min(256, targetCycle);

        for (auto pixelIndex = scanlineCycle_; pixelIndex < maxIndex; pixelIndex++)
        {
            background_.Load(pixelIndex);
        }

        if (enableBackground_)
        {
            for (auto pixelIndex = scanlineCycle_; pixelIndex < maxIndex; pixelIndex++)
            {
                scanlineData_[pixelIndex] = background_.Render();
                background_.Tick(pixelIndex);
            }
        }
        else
        {
            for (auto pixelIndex = scanlineCycle_; pixelIndex < maxIndex; pixelIndex++)
            {
                background_.Tick(pixelIndex);
            }
        }

        if (enableForeground_)
        {
            for (auto pixelIndex = scanlineCycle_; pixelIndex < maxIndex; pixelIndex++)
            {
                auto spritePixel = sprites_.RenderTick(scanlineData_[pixelIndex] > 0);
                if (spritePixel > 0)
                    scanlineData_[pixelIndex] = spritePixel;

                sprites_.EvaluationTick(currentScanline_, pixelIndex);
            }
        }
        else
        {
            for (auto pixelIndex = scanlineCycle_; pixelIndex < maxIndex; pixelIndex++)
            {
                sprites_.EvaluationTick(currentScanline_, pixelIndex);
            }
        }

        scanlineCycle_ = maxIndex;
        if (targetCycle_ <= 256)
            return;
    }

    if (scanlineCycle_ == 256)
    {
        for (auto pixel : scanlineData_)
        {
            display_.WritePixel(palette_[pixel]);
        }

        scanlineData_.fill(0);

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

    if (enableRendering_)
    {
        while (scanlineCycle_ < 320)
        {
            sprites_.LoadTick(currentScanline_, scanlineCycle_);

            scanlineCycle_++;
            if (scanlineCycle_ == targetCycle)
                return;
        }

        while (scanlineCycle_ < 336)
        {
            background_.Load(scanlineCycle_);
            background_.Tick(scanlineCycle_);

            scanlineCycle_++;
            if (scanlineCycle_ == targetCycle)
                return;
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
