#include "PpuBackground.h"
#include "PpuBackgroundState.h"
#include "Bus.h"

PpuBackground::PpuBackground(Bus& bus) :
    bus_{ bus }
{
}

void PpuBackground::SetBasePatternAddress(uint16_t address)
{
    state_.BackgroundPatternBase = address;
}

uint16_t PpuBackground::GetBasePatternAddress() const
{
    return state_.BackgroundPatternBase;
}

void PpuBackground::SetFineX(uint8_t value)
{
    patternBitShift_ += state_.FineX - value;
    patternBitShift_ &= 7;

    state_.FineX = value;
}

void PpuBackground::EnableLeftColumn(bool enabled)
{
    state_.LeftCrop = enabled ? 0 : 8;
}

void PpuBackground::EnableRendering(int32_t cycle)
{
    enabledCycle_ = cycle;

    if (cycle <= 0)
    {
        // this happens between scanlines.
        loadingIndex_ = 2;
    }
    else if (cycle >= 320)
    {
        loadingIndex_ = (cycle - 320) / 8;
    }
    else if (cycle < 256)
    {
        loadingIndex_ = (cycle + 16) / 8;
    }
}

void PpuBackground::BeginScanline()
{
    enabledCycle_ = 0;
    currentTileIndex_ = 0;
    currentTile_ = scanlineTiles_[0];
}

uint8_t PpuBackground::Render()
{
    auto patternBytes = currentTile_.PatternBytes >> patternBitShift_;
    auto index = (uint8_t)(
        ((patternBytes >> 7) & 2) |
        (patternBytes & 1));

    // this looks inefficient but compiles to a cmov instead of a conditional jump
    index |= currentTile_.AttributeBits; // palette
    return index & 3 ? index : 0;
}

void PpuBackground::RunRender(uint32_t startCycle, uint32_t endCycle)
{
    if (startCycle < state_.LeftCrop)
    {
        auto cropSize = std::min(state_.LeftCrop, endCycle);
        for (auto pixelIndex = startCycle; pixelIndex < cropSize; pixelIndex++)
        {
            backgroundPixels_[pixelIndex] = 0;
            Tick(pixelIndex);
        }

        startCycle = cropSize;
    }

    for (auto pixelIndex = startCycle; pixelIndex < endCycle; pixelIndex++)
    {
        backgroundPixels_[pixelIndex] = Render();
        Tick(pixelIndex);
    }
}

void PpuBackground::RunRenderDisabled(uint32_t startCycle, uint32_t endCycle)
{
    auto paletteIndex = 0;

    // if the PPU is pointing at a pallette entry, that replaces the background colour.
    if ((state_.CurrentAddress & 0x3f00) == 0x3f00)
    {
        paletteIndex = state_.CurrentAddress & 0x1f;
    }

    // TODO: this can be optimized
    for (auto pixelIndex = startCycle; pixelIndex < endCycle; pixelIndex++)
    {
        backgroundPixels_[pixelIndex] = paletteIndex;
        Tick(pixelIndex);
    }
}

void PpuBackground::RenderScanline()
{
    auto pixelIndex = 0;
    auto tileIndex = 0;

    if (state_.LeftCrop)
    {
        *reinterpret_cast<uint64_t *>(&backgroundPixels_[0]) = 0;
        pixelIndex = 8;
        tileIndex = 1;
    }

    uint8_t index;

    // push out first tile
    auto tile = scanlineTiles_[tileIndex++];
    auto patternBytes = tile.PatternBytes;
    auto attributeBits = tile.AttributeBits;

    switch (patternBitShift_)
    {
    case 7:
        index = (uint8_t)(attributeBits | ((patternBytes >> 14) & 2) | ((patternBytes >> 7) & 1));
        backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
        [[fallthrough]];

    case 6:
        index = (uint8_t)(attributeBits | ((patternBytes >> 13) & 2) | ((patternBytes >> 6) & 1));
        backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
        [[fallthrough]];

    case 5:
        index = (uint8_t)(attributeBits | ((patternBytes >> 12) & 2) | ((patternBytes >> 5) & 1));
        backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
        [[fallthrough]];

    case 4:
        index = (uint8_t)(attributeBits | ((patternBytes >> 11) & 2) | ((patternBytes >> 4) & 1));
        backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
        [[fallthrough]];

    case 3:
        index = (uint8_t)(attributeBits | ((patternBytes >> 10) & 2) | ((patternBytes >> 3) & 1));
        backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
        [[fallthrough]];

    case 2:
        index = (uint8_t)(attributeBits | ((patternBytes >> 9) & 2) | ((patternBytes >> 2) & 1));
        backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
        [[fallthrough]];

    case 1:
        index = (uint8_t)(attributeBits | ((patternBytes >> 8) & 2) | ((patternBytes >> 1) & 1));
        backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
        [[fallthrough]];

    case 0:
        index = (uint8_t)(attributeBits | ((patternBytes >> 7) & 2) | (patternBytes & 1));
        backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
    }

    // now render 31 tiles completely
    Tile prevTile{};
    uint64_t tileBytes{};
    for (; tileIndex < 32; tileIndex++)
    {
        tile = scanlineTiles_[tileIndex];
        if (tile != prevTile)
        {
            uint64_t tileLowBits = tile.PatternBytes;
            tileLowBits = (tileLowBits | (tileLowBits << 36)) & 0x000000f0000000f0;
            tileLowBits = (tileLowBits | (tileLowBits << 18)) & 0x00c000c000c000c0;
            tileLowBits = (tileLowBits | (tileLowBits << 9)) & 0x8080808080808080;

            uint64_t tileHighBits = tile.PatternBytes >> 8;
            tileHighBits = (tileHighBits | (tileHighBits << 36)) & 0x000000f0000000f0;
            tileHighBits = (tileHighBits | (tileHighBits << 18)) & 0x00c000c000c000c0;
            tileHighBits = (tileHighBits | (tileHighBits << 9)) & 0x8080808080808080;

            tileBytes = ((tileHighBits >> 6) | (tileLowBits >> 7));
            if (tile.AttributeBits)
            {
                uint64_t attributeBits = tile.AttributeBits;
                attributeBits |= (attributeBits << 8);
                attributeBits |= (attributeBits << 16);
                attributeBits |= (attributeBits << 32);

                uint64_t attributeMask = tileLowBits | tileHighBits;
                attributeMask |= attributeMask >> 1;
                attributeMask >>= 4;

                tileBytes |= (attributeBits & attributeMask);
            }

            prevTile = tile;
        }

        *((uint64_t*)(&backgroundPixels_[pixelIndex])) = tileBytes;
        pixelIndex += 8;
    }

    // and finally render the last bit of the last tile
    tile = scanlineTiles_[32];
    patternBytes = tile.PatternBytes;
    attributeBits = tile.AttributeBits;

    if (pixelIndex == 256)
        return;

    index = (uint8_t)(attributeBits | ((patternBytes >> 14) & 2) | ((patternBytes >> 7) & 1));
    backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
    if (pixelIndex == 256)
        return;

    index = (uint8_t)(attributeBits | ((patternBytes >> 13) & 2) | ((patternBytes >> 6) & 1));
    backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
    if (pixelIndex == 256)
        return;

    index = (uint8_t)(attributeBits | ((patternBytes >> 12) & 2) | ((patternBytes >> 5) & 1));
    backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
    if (pixelIndex == 256)
        return;

    index = (uint8_t)(attributeBits | ((patternBytes >> 11) & 2) | ((patternBytes >> 4) & 1));
    backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
    if (pixelIndex == 256)
        return;

    index = (uint8_t)(attributeBits | ((patternBytes >> 10) & 2) | ((patternBytes >> 3) & 1));
    backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
    if (pixelIndex == 256)
        return;

    index = (uint8_t)(attributeBits | ((patternBytes >> 9) & 2) | ((patternBytes >> 2) & 1));
    backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;
    if (pixelIndex == 256)
        return;

    index = (uint8_t)(attributeBits | ((patternBytes >> 8) & 2) | ((patternBytes >> 1) & 1));
    backgroundPixels_[pixelIndex++] = index & 3 ? index : 0;

    // we never need the last pixel
}

void PpuBackground::RunLoad(int32_t startCycle, int32_t endCycle)
{
    if (startCycle == 0 && endCycle == 256)
    {
        RunLoad();
        return;
    }

    auto cycle = startCycle;
    switch (cycle & 0x07)
    {
    case 0:
        while (true)
        {
            cycle++;
            if (cycle >= endCycle)
                return;
            [[fallthrough]];

    case 1:
            {
                auto tileAddress = (uint16_t)(0x2000 | state_.CurrentAddress & 0x0fff);
                nextTileId_ = bus_.PpuRead(tileAddress);
            }

            // Lock this in now - if the base address changes after this it doesn't take effect until the next tile.

            // address is 000PTTTTTTTT0YYY
            patternAddress_ = (uint16_t)
                (state_.BackgroundPatternBase | // pattern selector
                (nextTileId_ << 4) |
                    (state_.CurrentAddress >> 12)); // fineY

            cycle++;
            [[fallthrough]];

    case 2:
            cycle++;
            if (cycle >= endCycle)
                return;
            [[fallthrough]];

    case 3:
            {
                auto attributeAddress = (uint16_t)(
                    0x2000 | (state_.CurrentAddress & 0x0C00) | // select table
                    0x03C0 | // attribute block at end of table
                    (state_.CurrentAddress >> 4) & 0x0038 | // 3 bits of tile y
                    (state_.CurrentAddress >> 2) & 0x0007); // 3 bits of tile x

                auto attributes = bus_.PpuRead(attributeAddress);

                // use one more bit of the tile x and y to get the quadrant
                attributes >>= ((state_.CurrentAddress & 0x0040) >> 4) | (state_.CurrentAddress & 0x0002);
                attributes &= 0x03;

                scanlineTiles_[loadingIndex_].AttributeBits = attributes << 2;
            }
            cycle++;
            [[fallthrough]];

    case 4:
            cycle++;
            if (cycle >= endCycle)
                return;
            [[fallthrough]];

    case 5:
            scanlineTiles_[loadingIndex_].PatternBytes = bus_.PpuRead(patternAddress_);
            cycle++;
            [[fallthrough]];

    case 6:
            cycle++;
            if (cycle >= endCycle)
                return;
            [[fallthrough]];

    case 7:
            // address is 000PTTTTTTTT1YYY
            scanlineTiles_[loadingIndex_].PatternBytes |= bus_.PpuRead((uint16_t)(patternAddress_ | 8)) << 8;

            // increment the x part of the address
            if ((state_.CurrentAddress & 0x001f) == 0x001f)
            {
                // reset the X co-ordinate
                state_.CurrentAddress &= 0xffe0;
                // swap the nametable X
                state_.CurrentAddress ^= 0x0400;
            }
            else
            {
                state_.CurrentAddress++;
            }

            loadingIndex_++;

            cycle++;
            if (cycle >= endCycle)
                return;
#pragma warning(suppress: 4468)
            [[fallthrough]];
        }
    }
}

void PpuBackground::RunLoad()
{
    while (true)
    {
        auto tileAddress = (uint16_t)(0x2000 | state_.CurrentAddress & 0x0fff);
        nextTileId_ = bus_.PpuRead(tileAddress);

        auto attributeAddress = (uint16_t)(
            0x2000 | (state_.CurrentAddress & 0x0C00) | // select table
            0x03C0 | // attribute block at end of table
            (state_.CurrentAddress >> 4) & 0x0038 | // 3 bits of tile y
            (state_.CurrentAddress >> 2) & 0x0007); // 3 bits of tile x

        auto attributes = bus_.PpuRead(attributeAddress);

        // use one more bit of the tile x and y to get the quadrant
        attributes >>= ((state_.CurrentAddress & 0x0040) >> 4) | (state_.CurrentAddress & 0x0002);
        attributes &= 0x03;

        scanlineTiles_[loadingIndex_].AttributeBits = attributes << 2;

        // address is 000PTTTTTTTT0YYY
        patternAddress_ = (uint16_t)
            (state_.BackgroundPatternBase | // pattern selector
            (nextTileId_ << 4) |
                (state_.CurrentAddress >> 12)); // fineY

        auto pattern = bus_.PpuReadChr16(patternAddress_);

        // low address is 000PTTTTTTTT1YYY
        // high address is 000PTTTTTTTT1YYY
        scanlineTiles_[loadingIndex_].PatternBytes = pattern;

        if (loadingIndex_ == 32)
        {
            bus_.PpuDummyTileFetch();
            return;
        }

        // increment the x part of the address
        if ((state_.CurrentAddress & 0x001f) == 0x001f)
        {
            // reset the X co-ordinate
            state_.CurrentAddress &= 0xffe0;
            // swap the nametable X
            state_.CurrentAddress ^= 0x0400;
        }
        else
        {
            state_.CurrentAddress++;
        }

        loadingIndex_++;
    }
}

void PpuBackground::Tick(int32_t cycle)
{
    if (patternBitShift_ == 0)
    {
        patternBitShift_ = 7;

        currentTileIndex_ = ((cycle) >> 3) + 1;

        currentTile_ = scanlineTiles_[currentTileIndex_];
    }
    else
    {
        patternBitShift_--;
    }
}

void PpuBackground::HReset(uint16_t initialAddress)
{
    // adjust y scroll
    state_.CurrentAddress += 0x1000;
    state_.CurrentAddress &= 0x7fff;

    if ((state_.CurrentAddress & 0x7000) == 0)
    {
        // move to the next row
        auto yBits = (state_.CurrentAddress & 0x03e0);
        if (yBits == 0x03a0)
        {
            // reset the Y co-ordinate
            state_.CurrentAddress &= 0x7c1f;
            // swap the nametable Y
            state_.CurrentAddress ^= 0x0800;
        }
        else if (yBits == 0x03e0)
        {
            state_.CurrentAddress &= 0x7c1f;
        }
        else
        {
            state_.CurrentAddress += 0x0020;
        }
    }

    state_.CurrentAddress &= 0xfbe0;
    state_.CurrentAddress |= (uint16_t)(initialAddress & 0x041f);

    loadingIndex_ = 0;
}

void PpuBackground::HResetRenderDisabled()
{
    loadingIndex_ = 0;
}

void PpuBackground::VReset(uint16_t initialAddress)
{
    state_.CurrentAddress &= 0x041f;
    state_.CurrentAddress |= (uint16_t)(initialAddress & 0xfbe0);
}

uint32_t PpuBackground::GetDummyReadCount() const
{
    if (enabledCycle_ <= 336)
        return 2;
    else if (enabledCycle_ <= 338)
        return 1;
    else
        return 0;
}

bool PpuBackground::IsTile3Attribute() const
{
    return false;
}

uint16_t PpuBackground::CurrentAddress() const
{
    return state_.CurrentAddress;
}

uint16_t& PpuBackground::CurrentAddress()
{
    return state_.CurrentAddress;
}

const std::array<uint8_t, 256>& PpuBackground::ScanlinePixels() const
{
    return backgroundPixels_;
}

void PpuBackground::CaptureState(PpuBackgroundState* state) const
{
    *state = state_;
}

void PpuBackground::RestoreState(const PpuBackgroundState& state)
{
    state_ = state;
}
