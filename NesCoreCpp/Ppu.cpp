#include "Ppu.h"

#include "Bus.h"
#include "Display.h"
#include "PpuState.h"

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
    return state_.FrameCount;
}

void Ppu::Tick3()
{
    state_.TargetCycle += 3;
}

void Ppu::Sync()
{
    Sync(state_.TargetCycle);
}

uint8_t Ppu::Read(uint16_t address)
{
    address &= 0x07;

    if (address == 0x02)
    {
        state_.AddressLatch = false;

        uint8_t status = 0;

        if (state_.InVBlank)
        {
            status |= 0x80;
            state_.InVBlank = false;
        }

        // if the status is read at the point we were about to signal a vblank, that can cause it to be skipped.
        state_.SignalVBlank = false;

        // the flag should be reset at the start of the pre-render row, we need to sync in that case
        if (sprites_.Sprite0Hit())
        {
            // the pre-render line will reset this but we haven't synced yet
            if (state_.CurrentScanline != 261)
                status |= 0x40;
        }
        else if (sprites_.Sprite0Visible())
        {
            Sync(state_.TargetCycle);

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
        Sync(state_.TargetCycle);
        auto data = state_.PpuData;

        // the address is aliased with the PPU background render address, so we use that.
        auto ppuAddress = (uint16_t)(background_.CurrentAddress() & 0x3fff);
        state_.PpuData = bus_.PpuRead(ppuAddress);

        if (ppuAddress >= 0x3f00)
        {
            data = state_.Palette[ppuAddress & 0x1f] & state_.GrayscaleMask;
        }

        background_.CurrentAddress() += state_.AddressIncrement;
        background_.CurrentAddress() &= 0x7fff;
        bus_.SetChrA12(background_.CurrentAddress() & 0x1000);
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
        Sync(state_.TargetCycle);

        auto wasVblankInterruptEnabled = state_.EnableVBlankInterrupt;

        // PPUCTRL flags
        state_.EnableVBlankInterrupt = (value & 0x80) != 0;
        sprites_.SetLargeSprites((value & 0x20) != 0);
        background_.SetBasePatternAddress((uint16_t)((value & 0x10) << 8));
        sprites_.SetBasePatternAddress((uint16_t)((value & 0x08) << 9));
        state_.AddressIncrement = (value & 0x04) != 0 ? 32 : 1;

        // set base nametable address
        state_.InitialAddress &= 0xf3ff;
        state_.InitialAddress |= (uint16_t)((value & 3) << 10);

        if (state_.EnableVBlankInterrupt && !wasVblankInterruptEnabled && state_.InVBlank)
        {
            state_.SignalVBlank = true;
            bus_.Schedule(1, SyncEvent::PpuStateUpdate);
        }

        return;
    }

    case 1:
        state_.Mask = value;
        state_.UpdateMask = true;
        bus_.Schedule(1, SyncEvent::PpuStateUpdate);
        return;

    case 3:
        sprites_.SetOamAddress(value);
        return;

    case 5:
        if (!state_.AddressLatch)
        {
            // The fine X takes effect immediately, so we need to sync
            Sync(state_.TargetCycle);

            state_.InitialAddress &= 0xffe0;
            state_.InitialAddress |= (uint8_t)(value >> 3);
            background_.SetFineX((uint8_t)(value & 7));
            state_.AddressLatch = true;
        }
        else
        {
            if (state_.TargetCycle > 256)
            {
                // we only need to sync if we are pending an HReset
                Sync(state_.TargetCycle);
            }

            state_.InitialAddress &= 0x8c1f;
            state_.InitialAddress |= (uint16_t)((value & 0xf8) << 2);
            state_.InitialAddress |= (uint16_t)((value & 0x07) << 12);
            state_.AddressLatch = false;
        }
        return;

    case 6:
        if (!state_.AddressLatch)
        {
            if (state_.TargetCycle > 256)
            {
                // we only need to sync if we are pending an HReset
                Sync(state_.TargetCycle);
            }

            state_.InitialAddress = (uint16_t)(((value & 0x3f) << 8) | (state_.InitialAddress & 0xff));
            state_.AddressLatch = true;
        }
        else
        {
            state_.InitialAddress = (uint16_t)((state_.InitialAddress & 0xff00) | value);
            // update the address in 3 cycles time.
            state_.UpdateBaseAddress = true;
            bus_.Schedule(1, SyncEvent::PpuStateUpdate);

            state_.AddressLatch = false;
        }
        return;

    case 7:
    {
        Sync(state_.TargetCycle);

        auto writeAddress = (uint16_t)(background_.CurrentAddress() & 0x3fff);
        if (writeAddress >= 0x3f00)
        {
            SyncComposite(state_.TargetCycle);

            value &= 0x3f;

            if ((writeAddress & 0x03) == 0)
            {
                writeAddress &= 0x000f;

                // zero colors are mirrored
                state_.Palette[writeAddress] = value;
                state_.Palette[writeAddress | 0x0010] = value;

                auto rgb = display_.GetPixel(value & state_.GrayscaleMask, state_.Emphasis);
                state_.RgbPalette[writeAddress] = rgb;
                state_.RgbPalette[writeAddress | 0x0010] = rgb;
            }
            else
            {
                writeAddress &= 0x0001f;

                state_.Palette[writeAddress] = value;
                state_.RgbPalette[writeAddress] = display_.GetPixel(value & state_.GrayscaleMask, state_.Emphasis);
            }
        }
        else
        {
            bus_.PpuWrite(writeAddress, value);
        }

        background_.CurrentAddress() += state_.AddressIncrement;
        background_.CurrentAddress() &= 0x7fff;
        bus_.SetChrA12(background_.CurrentAddress() & 0x1000);
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

void Ppu::CaptureState(PpuState* state) const
{
    state->Core = state_;
    background_.CaptureState(&state->Background);
    sprites_.CaptureState(&state->Sprites);
}

void Ppu::RestoreState(const PpuState& state)
{
    state_ = state.Core;
    background_.RestoreState(state.Background);
    sprites_.RestoreState(state.Sprites);
}

void Ppu::SyncState()
{
    if (state_.SignalVBlank)
    {
        SignalVBlank();
        state_.SignalVBlank = false;
    }

    if (state_.UpdateBaseAddress)
    {
        Sync(state_.TargetCycle);
        bus_.SetChrA12((state_.InitialAddress & 0x1000) != 0);
        background_.CurrentAddress() = state_.InitialAddress;
        state_.UpdateBaseAddress = false;
    }
    else if (state_.UpdateMask)
    {
        // mask updates after 2 cycles
        Sync(state_.TargetCycle - 1);

        background_.EnableLeftColumn((state_.Mask & 0x02) != 0);
        sprites_.EnableLeftColumn((state_.Mask & 0x04) != 0);
        state_.EnableBackground = (state_.Mask & 0x08) != 0;
        state_.EnableForeground = (state_.Mask & 0x10) != 0;

        auto prevRenderingEnabled = state_.EnableRendering;
        state_.EnableRendering = (state_.Mask & 0x18) != 0;

        if (state_.EnableRendering && !prevRenderingEnabled && (state_.CurrentScanline < 240))
        {
            background_.EnableRendering(state_.TargetCycle - 1);
        }

        state_.GrayscaleMask = (state_.Mask & 0x01) ? 0x30 : 0xff;
        auto newEmphasis = (state_.Mask & 0xe0) >> 5;
        if (newEmphasis != state_.Emphasis)
        {
            SyncComposite(state_.TargetCycle - 1);
            state_.Emphasis = newEmphasis;

            // rebuild palette
            for (auto index = 0; index < 32; index++)
            {
                state_.RgbPalette[index] = display_.GetPixel(state_.Palette[index] & state_.GrayscaleMask, state_.Emphasis);
            }
        }

        state_.UpdateMask = false;
    }
}

void Ppu::SyncA12()
{
    // in the simple case the only effect we care about here is that the flag toggles, and we don't need to sync.
    if (!sprites_.LargeSprites())
    {
        bus_.SetChrA12(sprites_.BasePatternAddress()!= 0);
        return;
    }

    Sync();
}

void Ppu::SyncScanline()
{
    if (state_.TargetCycle < 340)
        return;

    // we're at the end of the scanline, so lets render the whole thing
    if (state_.CurrentScanline < 240)
    {
        if (state_.ScanlineCycle == 0)
        {
            RenderScanline();
        }
        else
        {
            RenderScanline(340);
        }

        state_.CurrentScanline++;
    }
    else if (state_.CurrentScanline == 240)
    {
        EnterVBlank();

        // if we've stepped over the start of VBlank, we should sync that too.
        if (state_.TargetCycle > 0)
        {
            SignalVBlank();
        }
        else
        {
            // otherwise we process the VBlank on the next update
            state_.SignalVBlank = true;
            bus_.Schedule(1, SyncEvent::PpuStateUpdate);
        }

        state_.CurrentScanline = 241;
    }
    else if (state_.CurrentScanline == 261)
    {
        PreRenderScanline(340);
        state_.CurrentScanline = 0;
    }
    else
    {
        state_.CurrentScanline++;
    }

    // the target cycle starts at -1 which gives us our extra tick at the start of the line
    state_.ScanlineCycle = 0;
    state_.TargetCycle -= 341;

    // For A12 we want to sync when we start the sprites.
    if (state_.EnableRendering && bus_.SensitiveToChrA12() && state_.CurrentScanline < 240)
        bus_.Schedule((259 - state_.TargetCycle) / 3, SyncEvent::PpuSyncA12);

    bus_.Schedule((342 - state_.TargetCycle) / 3, SyncEvent::PpuScanline);
}

void Ppu::Sync(int32_t targetCycle)
{
    if (targetCycle <= state_.ScanlineCycle)
        return;

#if DIAGNOSTIC
    diagnosticOverlay_[targetCycle] = 0xffff0000;
#endif

    if (state_.CurrentScanline < 240)
    {
        RenderScanline(targetCycle);
    }
    else if (state_.CurrentScanline == 261)
    {
        PreRenderScanline(targetCycle);
        return;
    }
    else
    {
        state_.ScanlineCycle = targetCycle;
    }
}

void Ppu::SyncComposite(int32_t targetCycle)
{
    if (state_.CurrentScanline >= 240)
        return;
    if (targetCycle < 0 || targetCycle > 256)
        return;

    Composite(state_.CompositeCycle, targetCycle);
    state_.CompositeCycle = targetCycle;
}

void Ppu::PreRenderScanline(int32_t targetCycle)
{
    if (state_.ScanlineCycle == 0)
    {
        background_.BeginScanline();
        sprites_.VReset();
        state_.InVBlank = false;
    }

    if (state_.ScanlineCycle < 256)
    {
        auto maxIndex = std::min(256, targetCycle);

        if (state_.EnableRendering)
        {
            background_.RunLoad(state_.ScanlineCycle, maxIndex);
        }

        state_.ScanlineCycle = maxIndex;
        if (state_.ScanlineCycle == targetCycle)
            return;
    }

    if (state_.ScanlineCycle == 256)
    {
        if (state_.EnableRendering)
            background_.HReset(state_.InitialAddress);
        else
            background_.HResetRenderDisabled();

        state_.ScanlineCycle = 257;
        if (state_.ScanlineCycle == targetCycle)
            return;
    }

    if (state_.ScanlineCycle < 320)
    {
        auto maxCycle = std::min(targetCycle, 320);

        sprites_.DummyLoad();

        if (state_.EnableRendering)
        {
            if (targetCycle >= 280 && state_.ScanlineCycle < 304)
            {
                background_.VReset(state_.InitialAddress);
            }

            state_.ScanlineCycle = maxCycle;
        }
        else
        {
            state_.ScanlineCycle = maxCycle;
        }

        if (state_.ScanlineCycle >= targetCycle)
            return;
    }

    if (state_.EnableRendering)
    {
        auto maxCycle = std::min(targetCycle, 336);

        // sprite tile loading
        background_.RunLoad(state_.ScanlineCycle, maxCycle);
    }

    state_.ScanlineCycle = targetCycle;
}

void Ppu::RenderScanline(int32_t targetCycle)
{
    if (state_.ScanlineCycle == 0)
    {
        goto RenderVisible;
    }

    if (state_.ScanlineCycle <= 256)
    {
        if (state_.ScanlineCycle < 256)
            RenderScanlineVisible(targetCycle);

        if (targetCycle <= 256)
            goto EndRender;

        FinishRender();

        if (!state_.EnableRendering)
            goto EndRender;

        goto LoadSprites;
    }

    if (!state_.EnableRendering)
        goto EndRender;

    if (state_.ScanlineCycle < 320)
    {
        if (targetCycle >= 320)
        {
            sprites_.RunLoad(state_.CurrentScanline, state_.ScanlineCycle, 320);
            goto LoadBackground;
        }
        else
        {
            sprites_.RunLoad(state_.CurrentScanline, state_.ScanlineCycle, targetCycle);
            goto EndRender;
        }
    }

    if (state_.ScanlineCycle < 336)
    {
        auto maxIndex = std::min(336, targetCycle);
        background_.RunLoad(state_.ScanlineCycle, maxIndex);
    }

    goto EndRender;

RenderVisible:
    background_.BeginScanline();

    if (targetCycle >= 256)
    {
        RenderScanlineVisible();

        if (targetCycle == 256)
            goto EndRender;

        FinishRender();
    }
    else
    {
        RenderScanlineVisible(targetCycle);
        goto EndRender;
    }

    if (!state_.EnableRendering)
        goto EndRender;

LoadSprites:
    if (targetCycle >= 320)
    {
        sprites_.RunLoad(state_.CurrentScanline, 256, 320);
    }
    else
    {
        sprites_.RunLoad(state_.CurrentScanline, 256, targetCycle);
        goto EndRender;
    }

LoadBackground:
    background_.RunLoad(320, targetCycle);

EndRender:
    state_.ScanlineCycle = targetCycle;
}

void Ppu::RenderScanlineVisible(int32_t targetCycle)
{
    auto maxIndex = std::min(256, targetCycle);

    if (state_.EnableRendering)
    {
        background_.RunLoad(state_.ScanlineCycle, maxIndex);

        if (state_.EnableBackground)
        {
            background_.RunRender(state_.ScanlineCycle, maxIndex);
        }
        else
        {
            background_.RunRenderDisabled(state_.ScanlineCycle, maxIndex);
        }

        if (state_.EnableForeground && state_.CurrentScanline != 0)
        {
            sprites_.RunRender(state_.ScanlineCycle, maxIndex, background_.ScanlinePixels());
        }

        sprites_.RunEvaluation(state_.CurrentScanline, state_.ScanlineCycle, maxIndex);
    }
    else
    {
        background_.RunRenderDisabled(state_.ScanlineCycle, maxIndex);
    }

    state_.ScanlineCycle = maxIndex;
}

void Ppu::RenderScanline()
{
    background_.BeginScanline();

    RenderScanlineVisible();

    FinishRender();

    if (state_.EnableRendering)
    {
        sprites_.RunLoad(state_.CurrentScanline);
        background_.RunLoad(320, 336);
    }
}

void Ppu::RenderScanlineVisible()
{
    // TODO: we can optimize the disabled case here
    if (state_.EnableRendering)
    {
        background_.RunLoad();

        if (state_.EnableBackground)
        {
            background_.RenderScanline();
        }
        else
        {
            background_.RunRenderDisabled(0, 256);
        }

        if (state_.EnableForeground && state_.CurrentScanline != 0)
        {
            sprites_.RunRender(0, 256, background_.ScanlinePixels());
        }

        sprites_.RunEvaluation(state_.CurrentScanline, 0, 256);
    }
    else
    {
        background_.RunRenderDisabled(0, 256);
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

            scanline[i] = state_.RgbPalette[pixel];
        }
    }
    else
    {
        for (auto i = startCycle; i < endCycle; i++)
        {
            auto pixel = backgroundPixels[i];
            scanline[i] = state_.RgbPalette[pixel];
        }
    }
}

void Ppu::FinishRender()
{
    Composite(state_.CompositeCycle, 256);
    state_.CompositeCycle = 0;

#if DIAGNOSTIC
    sprites_.MarkSprites(&diagnosticOverlay_[0]);

    auto pDisplay = display_.GetScanlinePtr();
    for (auto i = 256; i < Display::WIDTH; i++)
    {
        pDisplay[i] = 0;
    }

    for (auto pixel : diagnosticOverlay_)
    {
        if (pixel)
            *pDisplay = pixel;

        pDisplay++;
    }

    diagnosticOverlay_.fill(0);
#endif

    sprites_.HReset();
    display_.HBlank();

    if (state_.EnableRendering)
        background_.HReset(state_.InitialAddress);
    else
        background_.HResetRenderDisabled();
}

#include "windows.h"
#include <string>

void Ppu::EnterVBlank()
{
    display_.VBlank();

    static int lastCycleCount = 0;

    OutputDebugString((std::to_string(bus_.CycleCount() - lastCycleCount) + "\n").c_str());

    lastCycleCount = bus_.CycleCount();

    bus_.OnFrame();

    state_.FrameCount++;
}

void Ppu::SignalVBlank()
{
    // The VBlank flag of the PPU is set at tick 1 (the second tick) of scanline 241
    state_.InVBlank = true;
    if (state_.EnableVBlankInterrupt)
        bus_.SignalNmi();
}
