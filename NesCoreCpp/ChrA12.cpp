#include "ChrA12.h"

#include "PpuBackground.h"
#include "PpuSprites.h"

#include <cassert>

ChrA12::ChrA12(const PpuBackground& background, const PpuSprites& sprites)
    : background_{ background },
    sprites_{ sprites }
{
}

int32_t ChrA12::NextEdgeCycle(int32_t cycle)
{
    if (cycle < 256)
    {
        if ((background_.CurrentAddress() & 0x1000) != 0)
        {
            // background loading will alternate between low and high every 4 cycles, starting low
            // (the nametable bytes)
            cycle &= ~3;
            cycle += 4;
            return cycle;
        }
        // otherwise, it will stay low the whole time

        cycle = 256;
    }

    if (cycle < 320)
    {
        if ((sprites_.BasePatternAddress() & 0x1000) != 0 || sprites_.LargeSprites())
        {
            // For large sprites we would need calculate this properly we will need to wait until sprite evaluation has
            // occurred and then determine the sprites in the high plane.
            // However, this is an edge case, so I'm not going to optimize for that.  We can just sync on every sprite.
            cycle &= ~3;
            cycle += 4;
            return cycle;
        }

        // otherwise, it will stay low the whole time
        cycle = 320;
    }

    if (cycle < 336)
    {
        if ((background_.CurrentAddress() & 0x1000) != 0)
        {
            // background loading will alternate between low and high every 4 cycles, starting low
            // (the nametable bytes)
            cycle &= ~3;
            cycle += 4;
            return cycle;
        }
        // otherwise, it will stay low the whole time
    }

    // and then it stays low for the rest of the scanline.
    return -1;
}

int32_t ChrA12::NextRaisingEdgeCycleFiltered(int32_t cycle, bool isLow)
{
    auto backgroundHigh = (background_.GetBasePatternAddress() & 0x1000) != 0;
    auto spritesHigh = (sprites_.BasePatternAddress() & 0x1000) != 0;

    if (isLow)
    {
        if (cycle < 256)
        {
            if (backgroundHigh)
            {
                cycle += 4;
                cycle &= ~7;
                cycle += 4;
                return cycle;
            }
            cycle = 256;
        }

        if (cycle < 320)
        {
            if (cycle < 316 && (sprites_.LargeSprites() || spritesHigh))
            {
                cycle += 4;
                cycle &= ~7;
                cycle += 4;
                return cycle;
            }
            cycle = 320;
        }

        if (cycle < 340)
        {
            if (backgroundHigh)
            {
                cycle += 4;
                cycle &= ~7;
                cycle += 4;
                return cycle;
            }
        }
    }
    if (sprites_.LargeSprites())
    {
        // we don't know where each sprite will be loaded from, so will have to assume the signal could be raised for any
        // of them.

        if (cycle < 256)
        {
            cycle = 256; // a raising edge won't be triggered by the background.
        }

        if (cycle < 320) // sprite loading
        {
            if (cycle < 316)
            {
                cycle += 4;
                cycle &= ~7;
                cycle += 4;
                return cycle;
            }

            cycle = 320;
        }

        // finally, this could trigger when we start to load the background again
        if (backgroundHigh && cycle < 324)
            return 324;
    }
    else
    {
        // The filtered signal will either trigger with the first sprite, or the first tile on the next scanline
        if (backgroundHigh == spritesHigh)
            return -1;

        if (cycle < 260 && spritesHigh)
        {
            return 260;
        }

        if (cycle < 324 && backgroundHigh)
        {
            return 324;
        }
    }

    return -1;
}

int32_t ChrA12::NextTrailingEdgeCycle(int32_t cycle)
{
    if (cycle < 256)
    {
        if ((background_.GetBasePatternAddress() & 0x1000) != 0)
        {
            // background loading will alternate between low and high every 4 cycles, starting low
            // (the nametable bytes)
            cycle &= ~7;
            cycle += 8;
            return cycle;
        }
        // otherwise, it will stay low the whole time

        cycle = 256;
    }

    if (cycle < 320)
    {
        if ((sprites_.BasePatternAddress() & 0x1000) != 0 || sprites_.LargeSprites())
        {
            // For large sprites we would need calculate this properly we will need to wait until sprite evaluation has
            // occurred and then determine the sprites in the high plane.
            // However, this is an edge case, so I'm not going to optimize for that.  We can just sync on every sprite.
            cycle &= ~7;
            cycle += 8;
            return cycle;
        }

        // otherwise, it will stay low the whole time
        cycle = 320;
    }

    if (cycle < 332)
    {
        if ((background_.GetBasePatternAddress() & 0x1000) != 0)
        {
            // background loading will alternate between low and high every 4 cycles, starting low
            // (the nametable bytes)
            cycle &= ~7;
            cycle += 8;
            return cycle;
        }
        // otherwise, it will stay low the whole time
    }

    // and then it stays low for the rest of the scanline.
    return -1;
}

SignalEdge ChrA12::GetEdge(int32_t& cycle, bool smoothed)
{
    assert((cycle & 0x2) == 0);

    if (cycle < 256 || cycle >= 320)
    {
        auto edge = cycle & 0x04 ? SignalEdge::Rising : SignalEdge::Falling;

        if (smoothed)
        {
            if (cycle < 256)
                cycle = 256;
            else
                cycle = 340;
        }

        return edge;
    }
    else
    {
        if (!sprites_.LargeSprites() || sprites_.AllLargeSpritesHighTable())
        {
            auto edge = cycle & 0x04 ? SignalEdge::Rising : SignalEdge::Falling;

            if (smoothed)
            {
                // skip the rest of the sprites
                cycle = 320;
            }

            return edge;
        }
        else
        {
            int currentSprite = (cycle - 256) / 8;

            if (!sprites_.IsHighTable(currentSprite))
                return SignalEdge::None;

            auto edge = cycle & 0x04 ? SignalEdge::Rising : SignalEdge::Falling;

            if (smoothed)
            {
                // skip over any other following high sprites as we want to smooth over these edges
                do
                {
                    ++currentSprite;
                    cycle += 8;
                } while (currentSprite < 8 && sprites_.IsHighTable(currentSprite));
            }

            return edge;
        }
    }
}
