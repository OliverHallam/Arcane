#include "Ppu.h"

#include "Bus.h"
#include "Display.h"
#include "PpuState.h"

#include <cassert>


Ppu::Ppu(Bus& bus, Display& display) :
    bus_{ bus },
    display_{ display },
    background_{ bus },
    sprites_{ bus },
    chrA12_{ background_, sprites_ }
{
    bus_.SchedulePpu(340, SyncEvent::PpuScanline);
}

uint32_t Ppu::FrameCount()
{
    return state_.FrameCount;
}

void Ppu::Sync()
{
    Sync(ScanlineCycle());
}

uint8_t Ppu::Read(uint16_t address)
{
    auto scanlineCycle = ScanlineCycle();
    auto registerAddress = address & 0x07;

    if (registerAddress == 0x02)
    {
        state_.AddressLatch = false;

        uint8_t status = 0;

        if (state_.InVBlank)
        {
            // This is in lieu of a sync - the flag would have got unset by 261/1
            if (state_.CurrentScanline != 261 || scanlineCycle < 0)
            {
                // Reading PPUSTATUS within two cycles of the start of vertical blank will return 0 in bit 7 but clear the latch anyway, causing NMI to not occur that frame.
                status |= 0x80;
            }
            
            state_.InVBlank = false;
        }

        if (state_.CurrentScanline == 241)
        {
            if (scanlineCycle <= 1)
            {
                if (scanlineCycle == -1)
                    state_.SuppressVBlank = true;

                // The NMI line is pulled back down before the CPU notices
                bus_.Deschedule(SyncEvent::CpuNmi);
            }
        }

        // the flag should be reset at the start of the pre-render row, we need to sync in that case
        if (sprites_.Sprite0Hit())
        {
            // the pre-render line will reset this but we haven't synced yet
            if (state_.CurrentScanline != 261)
                status |= 0x40;
        }
        else if (sprites_.Sprite0Visible())
        {
            Sync(scanlineCycle);

            if (sprites_.Sprite0Hit())
            {
                status |= 0x40;
            }
        }

        if (sprites_.SpriteOverflow())
            status |= 0x20;

        return status;
    }
    else if (registerAddress == 0x07)
    {
        Sync(scanlineCycle);
        auto data = state_.PpuData;

        // the address is aliased with the PPU background render address, so we use that.
        auto ppuAddress = (uint16_t)(background_.CurrentAddress() & 0x3fff);
        state_.PpuData = bus_.PpuRead(ppuAddress);

        // this can trip up the "end-of-frame" detection in the scanline detector, so delay that if it's in progress
        if (bus_.HasScanlineCounter())
        {
            if (bus_.Deschedule(SyncEvent::ScanlineCounterEndFrame))
            {
                // TODO: check the number here
                bus_.Schedule(3, SyncEvent::ScanlineCounterEndFrame);
            }
        }

        if (ppuAddress >= 0x3f00)
        {
            data = state_.Palette[ppuAddress & 0x1f] & state_.GrayscaleMask;
        }

        SetCurrentAddress((background_.CurrentAddress() + state_.AddressIncrement) & 0x7fff);

        return data;
    }
    else if (registerAddress == 0x04)
    {
        // If we call this during sprite loading then we intercept the values being read there instead of at the OAM
        // address.
        if (state_.EnableRendering && state_.CurrentScanline < 256)
        {
            // while OAM is being cleared, the bus is forced to 0xff
            if (static_cast<uint32_t>(scanlineCycle) < 64)
                return 0xff;

            if (scanlineCycle >= 256 && scanlineCycle < 320)
            {
                // sync to end of sprite evaluation to ensure the OAM is populated
                Sync(256);

                // work out the value that the hardware would be reading at our current cycle.
                return sprites_.GetLoadingOamData(scanlineCycle);
            }
        }

        return sprites_.ReadOam();
    }

    return 0;
}

void Ppu::Write(uint16_t address, uint8_t value)
{
    auto targetCycle = ScanlineCycle();

    switch (address & 0x07)
    {
    case 0:
    {
        Sync(targetCycle);

        auto wasVblankInterruptEnabled = state_.EnableVBlankInterrupt;

        // PPUCTRL flags
        state_.EnableVBlankInterrupt = (value & 0x80) != 0;
        auto largeSprites = (value & 0x20) != 0;
        sprites_.SetLargeSprites(largeSprites);
        background_.SetBasePatternAddress((uint16_t)((value & 0x10) << 8));
        sprites_.SetBasePatternAddress((uint16_t)((value & 0x08) << 9));
        state_.AddressIncrement = (value & 0x04) != 0 ? 32 : 1;

        // set base nametable address
        state_.InitialAddress &= 0xf3ff;
        state_.InitialAddress |= (uint16_t)((value & 3) << 10);

        if (state_.InVBlank && state_.EnableVBlankInterrupt != wasVblankInterruptEnabled)
        {
            if (state_.EnableVBlankInterrupt)
            {
                // if we call this just before the VBlank flag gets cleared, we toggle the NMI signal off quick enough
                // that the CPU doesn't see it
                if (state_.CurrentScanline != 261)
                    bus_.SignalNmi();
            }
            else
            {
                // if we call this just after the VBlank flag gets set, we toggle the NMI signal off quick enough
                // that the CPU doensn't see it
                if (state_.CurrentScanline == 241 && targetCycle < 2)
                    bus_.Deschedule(SyncEvent::CpuNmi);
            }
        }

        if (address == 0x2000) // doesn't apply to mirrors
            bus_.InterceptPpuCtrl(largeSprites);

#ifdef DIAGNOSTIC
        MarkDiagnostic(0xffffff00);
#endif

        return;
    }

    case 1:
        state_.Mask = value;
        state_.UpdateMask = true;
        bus_.SchedulePpu(2, SyncEvent::PpuStateUpdate);

        if (address == 0x2001)
        {
            bus_.InterceptPpuMask((value & 0x18) != 0);
        }

#ifdef DIAGNOSTIC
        MarkDiagnostic(0xffff00ff);
#endif
        return;

    case 3:
        sprites_.SetOamAddress(value);
        return;

    case 5:
        if (!state_.AddressLatch)
        {
            // The fine X takes effect immediately, so we need to sync
            Sync(targetCycle);

            state_.InitialAddress &= 0xffe0;
            state_.InitialAddress |= (uint8_t)(value >> 3);
            background_.SetFineX((uint8_t)(value & 7));
            state_.AddressLatch = true;
        }
        else
        {
            if (targetCycle > 256)
            {
                // we only need to sync if we are pending an HReset
                Sync(targetCycle);
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
            if (targetCycle > 256)
            {
                // we only need to sync if we are pending an HReset
                Sync(targetCycle);
            }

            state_.InitialAddress = (uint16_t)(((value & 0x3f) << 8) | (state_.InitialAddress & 0xff));
            state_.AddressLatch = true;
        }
        else
        {
            state_.InitialAddress = (uint16_t)((state_.InitialAddress & 0xff00) | value);
            // update the address in 3 cycles time.
            state_.UpdateBaseAddress = true;
            bus_.SchedulePpu(3, SyncEvent::PpuStateUpdate);

            state_.AddressLatch = false;
        }
        return;

    case 7:
    {
        Sync(targetCycle);

        auto writeAddress = (uint16_t)(background_.CurrentAddress() & 0x3fff);
        if (writeAddress >= 0x3f00)
        {
            SyncComposite(targetCycle);

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

        SetCurrentAddress((background_.CurrentAddress() + state_.AddressIncrement) & 0x7fff);

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

void Ppu::DmaCompleted()
{
    sprites_.OamDmaCompleted();
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

void Ppu::UpdateA12Sensitivity(bool isLow)
{
    // TODO: we need to sort out the smoothing here - this is a function of when A12 was last high.
    if (state_.CurrentScanline < 240 || state_.CurrentScanline == 261)
    {
        bus_.Deschedule(SyncEvent::PpuSyncA12);
        ScheduleA12Sync(ScanlineCycle(), isLow);
    }
}

#ifdef DIAGNOSTIC
void Ppu::MarkDiagnostic(uint32_t color)
{
    auto scanlineCycle = ScanlineCycle();
    diagnosticOverlay_[scanlineCycle + 1] = color;
}
#endif

void Ppu::ScheduleA12Sync(int32_t cycle, bool isLow)
{
    if (!state_.EnableRendering)
        return;

    int32_t edgeCycle = -1;
    switch (bus_.ChrA12Sensitivity())
    {
    case ChrA12Sensitivity::RisingEdgeSmoothed:
        edgeCycle = chrA12_.NextRaisingEdgeCycleFiltered(cycle, isLow);
        break;

    case ChrA12Sensitivity::FallingEdgeDivided:
        edgeCycle = chrA12_.NextTrailingEdgeCycle(cycle);
        break;

    case ChrA12Sensitivity::AllEdges:
        edgeCycle = chrA12_.NextEdgeCycle(cycle);
        break;
    }

    if (edgeCycle >= 0)
    {
        bus_.SchedulePpu(edgeCycle - ScanlineCycle(), SyncEvent::PpuSyncA12);
    }
}

void Ppu::SyncState()
{
    auto scanlineCycle = ScanlineCycle();

    if (state_.UpdateBaseAddress)
    {
        // TODO: reschedule A12 if rendering
        Sync(scanlineCycle);
        SetCurrentAddress(state_.InitialAddress);
        state_.UpdateBaseAddress = false;
    }
    else if (state_.UpdateMask)
    {
        Sync(scanlineCycle);

        background_.EnableLeftColumn((state_.Mask & 0x02) != 0);
        sprites_.EnableLeftColumn((state_.Mask & 0x04) != 0);
        state_.EnableBackground = (state_.Mask & 0x08) != 0;
        state_.EnableForeground = (state_.Mask & 0x10) != 0;

        auto prevRenderingEnabled = state_.EnableRendering;
        state_.EnableRendering = (state_.Mask & 0x18) != 0;

        if (state_.EnableRendering != prevRenderingEnabled && (state_.CurrentScanline < 240))
        {
            if (state_.EnableRendering)
                background_.EnableRendering(scanlineCycle - 1);
            else if (bus_.HasScanlineCounter())
            {
                // we will stop reading memory, so cancel any pending scanline counter events.
                if (scanlineCycle < 5)
                {
                    // we could have two events in the queue
                    bus_.DescheduleAll(SyncEvent::ScanlineCounterScanline);
                }

                // and if no memory is read in the next 3 cycles, the scanline detector will register an end-of-frame.
                bus_.Schedule(3, SyncEvent::ScanlineCounterEndFrame);
            }

            // restart the A12 sync
            UpdateA12Sensitivity(true);
        }

        state_.GrayscaleMask = (state_.Mask & 0x01) ? 0x30 : 0xff;
        auto newEmphasis = (state_.Mask & 0xe0) >> 5;
        if (newEmphasis != state_.Emphasis)
        {
            SyncComposite(scanlineCycle - 1);
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
    if (bus_.ChrA12Sensitivity() == ChrA12Sensitivity::None || !state_.EnableRendering)
        return; // we don't care any more

    auto edgeCycle = ScanlineCycle();

    if (sprites_.LargeSprites() && !sprites_.AllLargeSpritesHighTable() && edgeCycle >= 256 && edgeCycle < 320 && state_.EnableForeground)
    {
        // we need the sprites to be loaded.
        Sync(256);
    }

    auto edge = chrA12_.GetEdge(edgeCycle, bus_.ChrA12Sensitivity() == ChrA12Sensitivity::RisingEdgeSmoothed);

    if (edge == SignalEdge::Rising)
    {
        bus_.ChrA12Rising();
    }
    else if (edge == SignalEdge::Falling)
        bus_.ChrA12Falling();

    ScheduleA12Sync(edgeCycle, false);
}

void Ppu::SyncScanline()
{
    assert(ScanlineCycle() == 340);

#if DIAGNOSTIC
    auto currentScanline = state_.CurrentScanline;
#endif

    // we're at the end of the scanline, so lets render the whole thing
    if (state_.CurrentScanline < 240)
    {
        if (state_.SyncCycle == 0)
        {
            RenderScanline();
        }
        else
        {
            RenderScanline(340);
        }

        state_.CurrentScanline++;

        // the scanline starts at -1 which gives us our extra tick at the start of the line
        state_.SyncCycle = -1;
        state_.ScanlineStartCycle = bus_.PpuCycleCount() + 1;
    }
    else if (state_.CurrentScanline == 240)
    {
        EnterVBlank();

        state_.CurrentScanline = 241;

        // the target starts at -1 which gives us our extra tick at the start of the line
        state_.SyncCycle = -1;
        state_.ScanlineStartCycle = bus_.PpuCycleCount() + 1;
    }
    else if (state_.CurrentScanline == 261)
    {
        PreRenderScanline(340);
        state_.CurrentScanline = 0;

        if (state_.EnableRendering && (state_.FrameCount & 1))
        {
            state_.SyncCycle = 0;
            state_.ScanlineStartCycle = bus_.PpuCycleCount();
        }
        else
        {
            state_.SyncCycle = -1;
            state_.ScanlineStartCycle = bus_.PpuCycleCount() + 1;
        }
    }
    else
    {
        state_.CurrentScanline++;

        // the target cycle starts at -1 which gives us our extra tick at the start of the line
        state_.SyncCycle = -1;
        state_.ScanlineStartCycle = bus_.PpuCycleCount() + 1;
    }


#if DIAGNOSTIC
    RenderOverlay(currentScanline);
    Clear(state_.CurrentScanline);
#endif

    if (state_.EnableRendering && (state_.CurrentScanline <= 240 || state_.CurrentScanline == 261))
    {
        if (state_.CurrentScanline == 240)
        {
            if (bus_.HasScanlineCounter())
            {
                // after 3 cycles without a read the "end frame" event should be triggered.
                bus_.Schedule(3, SyncEvent::ScanlineCounterEndFrame);
            }
        }
        else
        {
            if (bus_.HasScanlineCounter())
            {
                // There's two sides to the MMC5 scanline counter.  The triggering of the start of the scanline can
                // cause an interrupt on the CPU so has to be scheduled carefully.  At the same time, this also
                // causes the MMC5 to start counting memory accesses and swap out the CHR memory appropriately

                // we need to trigger the MMC5 scanline counter if we access the nametable for the same address 3 times
                // in a row.  We usually see two dummy accesses for the current tile at the end of the previous
                // scanline- the first tile hits this again, and then the next memory access (on cycle 2) will trip the
                // detector

                // In the hardware these events are the same thing, but we are emulating these in two different
                // timelines (PPU time and CPU time) so need to track them seperately.

                // There's an edge case if the first tile is pointing to attribute memorwhich can cause it to trip
                // twice so we may need to schedule this event twice, 2 PPU cycles apart
                auto dummyReads = background_.GetDummyReadCount();
                auto nextTileIsAttribute = (background_.CurrentAddress() & 0x03ff) >= 0x03c0;

                // First let's schedule the Scanline Counter events
                if (dummyReads == 2)
                {
                    bus_.SchedulePpu(2, SyncEvent::ScanlineCounterScanline);
                }

                // if the current tile is pointing at attribute memory, this can trigger an extra scanline
                if (dummyReads > 1 && nextTileIsAttribute)
                {
                    bus_.SchedulePpu(4, SyncEvent::ScanlineCounterScanline);
                }

                // and then start the split counter for our lazy timeline
                bus_.TileSplitBeginScanline(nextTileIsAttribute);
            }

            ScheduleA12Sync(0, state_.CurrentScanline == 261);
        }
    }

    if (state_.CurrentScanline == 241)
    {
        // trigger the NMI if necessary.
        // TODO: we could skip this when NMI is disabled
        bus_.SchedulePpu(1, SyncEvent::PpuSync);
    }

    bus_.SchedulePpu(340 - state_.SyncCycle, SyncEvent::PpuScanline);
}

int32_t Ppu::ScanlineCycle() const
{
    return static_cast<int32_t>(bus_.PpuCycleCount() - state_.ScanlineStartCycle);
}

void Ppu::Sync(int32_t targetCycle)
{
    if (targetCycle <= state_.SyncCycle)
        return;

#if DIAGNOSTIC
    diagnosticOverlay_[targetCycle] = 0xffff0000;
#endif

    if (state_.CurrentScanline < 240)
    {
        RenderScanline(targetCycle);
    }
    else if (state_.CurrentScanline == 241)
    {
        if (state_.SyncCycle <= 0 && targetCycle >= 0)
        {
            if (state_.SuppressVBlank)
                state_.SuppressVBlank = false;
            else
            {
                state_.InVBlank = true;
                assert(targetCycle == 0);
                if (state_.EnableVBlankInterrupt)
                    bus_.SignalNmi();
            }
        }
        state_.SyncCycle = 340; // nothing interesting can happen this scanline
    }
    else if (state_.CurrentScanline == 261)
    {
        PreRenderScanline(targetCycle);
        return;
    }
    else
    {
        state_.SyncCycle = targetCycle;
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
    if (state_.SyncCycle <= 0)
    {
        state_.SyncCycle = 0;

        background_.BeginScanline();
        sprites_.VReset();
        state_.InVBlank = false;
    }

    if (state_.SyncCycle < 256)
    {
        auto maxIndex = std::min(256, targetCycle);

        if (state_.EnableRendering)
        {
            background_.RunLoad(state_.SyncCycle, maxIndex);
        }

        state_.SyncCycle = maxIndex;
        if (state_.SyncCycle == targetCycle)
            return;
    }

    if (state_.SyncCycle == 256)
    {
        if (state_.EnableRendering)
            background_.HReset(state_.InitialAddress);
        else
            background_.HResetRenderDisabled();

        state_.SyncCycle = 257;
        if (state_.SyncCycle == targetCycle)
            return;
    }

    if (state_.SyncCycle < 320)
    {
        auto maxCycle = std::min(targetCycle, 320);

        if (state_.EnableRendering)
        {
            if (targetCycle >= 280 && state_.SyncCycle < 304)
            {
                background_.VReset(state_.InitialAddress);
            }

            state_.SyncCycle = maxCycle;
        }
        else
        {
            state_.SyncCycle = maxCycle;
        }

        if (state_.SyncCycle >= targetCycle)
            return;
    }

    if (state_.EnableRendering)
    {
        auto maxCycle = std::min(targetCycle, 336);

        // sprite tile loading
        background_.RunLoad(state_.SyncCycle, maxCycle);
    }

    state_.SyncCycle = targetCycle;
}

void Ppu::RenderScanline(int32_t targetCycle)
{
    if (state_.SyncCycle <= 0)
    {
        state_.SyncCycle = 0;
        goto RenderVisible;
    }

    if (state_.SyncCycle <= 256)
    {
        if (state_.SyncCycle < 256)
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

    if (state_.SyncCycle < 320)
    {
        if (targetCycle >= 320)
        {
            sprites_.RunLoad(state_.CurrentScanline, state_.SyncCycle, 320);
            goto LoadBackground;
        }
        else
        {
            sprites_.RunLoad(state_.CurrentScanline, state_.SyncCycle, targetCycle);
            goto EndRender;
        }
    }

    if (state_.SyncCycle < 336)
    {
        auto maxIndex = std::min(336, targetCycle);
        background_.RunLoad(state_.SyncCycle, maxIndex);
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
    state_.SyncCycle = targetCycle;
}

void Ppu::RenderScanlineVisible(int32_t targetCycle)
{
    auto maxIndex = std::min(256, targetCycle);

    if (state_.EnableRendering)
    {
        background_.RunLoad(state_.SyncCycle, maxIndex);

        if (state_.EnableBackground)
        {
            background_.RunRender(state_.SyncCycle, maxIndex);
        }
        else
        {
            background_.RunRenderDisabled(state_.SyncCycle, maxIndex);
        }

        if (state_.EnableForeground && state_.CurrentScanline != 0)
        {
            sprites_.RunRender(state_.SyncCycle, maxIndex, background_.ScanlinePixels());
        }

        sprites_.RunEvaluation(state_.CurrentScanline, state_.SyncCycle, maxIndex);
    }
    else
    {
        background_.RunRenderDisabled(state_.SyncCycle, maxIndex);
    }

    state_.SyncCycle = maxIndex;
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
#endif

    sprites_.HReset();
    display_.HBlank();

    if (state_.EnableRendering)
        background_.HReset(state_.InitialAddress);
    else
        background_.HResetRenderDisabled();
}

#if DIAGNOSTIC

void Ppu::Clear(int32_t scanline)
{
    auto pDisplay = display_.GetScanlinePtr(scanline);
    for (auto i = 0; i < Display::WIDTH; i++)
    {
        pDisplay[i] = 0;
    }

    diagnosticOverlay_.fill(0);
}

void Ppu::RenderOverlay(int32_t scanline)
{
    auto pDisplay = display_.GetScanlinePtr(scanline);
    for (auto pixel : diagnosticOverlay_)
    {
        if (pixel)
            *pDisplay = pixel;

        pDisplay++;
    }
}

#endif

void Ppu::EnterVBlank()
{
    display_.VBlank();

    bus_.OnFrame();

    state_.FrameCount++;
}

void Ppu::SetCurrentAddress(uint16_t address)
{
    auto a12Before = (background_.CurrentAddress() & 0x1000) != 0;
    background_.CurrentAddress() = address;
    auto a12After = (background_.CurrentAddress() & 0x1000) != 0;

    // doesn't take effect during rendering
    if (state_.EnableRendering && (state_.CurrentScanline < 240 || state_.CurrentScanline == 261))
    {
        return;
    }

    if (a12Before != a12After)
    {
        if (a12After)
            bus_.ChrA12Rising();
        else
            bus_.ChrA12Falling();
    }
}

