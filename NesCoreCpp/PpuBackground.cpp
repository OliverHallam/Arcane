#include "PpuBackground.h"
#include "Bus.h"

PpuBackground::PpuBackground(const Bus& bus) :
    bus_{ bus }
{
}

void PpuBackground::SetBasePatternAddress(uint16_t address)
{
    backgroundPatternBase_ = address;
}

void PpuBackground::SetFineX(uint8_t value)
{
    fineX_ = value;
}

int8_t PpuBackground::Render()
{
    auto index = (uint8_t)(
        ((currentTile_.PatternByteHigh >> patternBitShift_) & 1) << 1) |
        ((currentTile_.PatternByteLow >> patternBitShift_) & 1);

    if (index == 0)
        return 0;

    index |= currentTile_.AttributeBits; // palette
    return index;
}

void PpuBackground::Tick(int32_t scanlineCycle)
{
    switch (scanlineCycle & 0x07)
    {
    case 0:
        nextTile_ = loadingTile_;

    case 1:
    {
        auto tileAddress = (uint16_t)(0x2000 | CurrentAddress & 0x0fff);
        nextTileId_ = bus_.PpuRead(tileAddress);
        break;
    }

    case 3:
    {
        auto attributeAddress = (uint16_t)(
            0x2000 | (CurrentAddress & 0x0C00) | // select table
            0x03C0 | // attribute block at end of table
            (CurrentAddress >> 4) & 0x0038 | // 3 bits of tile y
            (CurrentAddress >> 2) & 0x0007); // 3 bits of tile x

        auto attributes = bus_.PpuRead(attributeAddress);

        // use one more bit of the tile x and y to get the quadrant
        attributes >>= ((CurrentAddress & 0x0040) >> 4) | (CurrentAddress & 0x0002);
        attributes &= 0x03;

        loadingTile_.AttributeBits = attributes << 2;
        break;
    }

    case 5:
        // address is 000PTTTTTTTT0YYY
        patternAddress_ = (uint16_t)
            (backgroundPatternBase_ | // pattern selector
            (nextTileId_ << 4) |
                (CurrentAddress >> 12)); // fineY

        loadingTile_.PatternByteLow = bus_.PpuRead(patternAddress_);
        break;

    case 7:
        // address is 000PTTTTTTTT1YYY
        loadingTile_.PatternByteHigh = bus_.PpuRead((uint16_t)(patternAddress_ | 8));

        if (scanlineCycle == 255)
        {
            // adjust y scroll
            CurrentAddress += 0x1000;
            CurrentAddress &= 0x7fff;

            if ((CurrentAddress & 0x7000) == 0)
            {
                // move to the next row
                if ((CurrentAddress & 0x03e0) == 0x03e0)
                {
                    // reset the X co-ordinate
                    CurrentAddress &= 0x7c1f;
                    // swap the nametable Y
                    CurrentAddress ^= 0x0800;
                }
                else
                {
                    CurrentAddress += 0x0020;
                }
            }
        }
        else
        {
            // increment the x part of the address
            if ((CurrentAddress & 0x001f) == 0x001f)
            {
                // reset the X co-ordinate
                CurrentAddress &= 0xffe0;
                // swap the nametable X
                CurrentAddress ^= 0x0400;
            }
            else
            {
                CurrentAddress++;
            }
        }
        break;
    }

    if (patternBitShift_ == 0)
    {
        patternBitShift_ = 7;
        currentTile_ = nextTile_;
    }
    else
    {
        patternBitShift_--;
    }
}

void PpuBackground::HReset(uint16_t initialAddress)
{
    CurrentAddress &= 0xfbe0;
    CurrentAddress |= (uint16_t)(initialAddress & 0x041f);

    patternBitShift_ =  7- fineX_;
}

void PpuBackground::VReset(uint16_t initialAddress)
{
    CurrentAddress &= 0x041f;
    CurrentAddress |= (uint16_t)(initialAddress & 0xfbe0);
}
