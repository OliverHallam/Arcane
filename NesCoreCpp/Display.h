#pragma once

#include <array>
#include <cstdint>

class Display
{
public:
    Display();

    uint32_t GetPixel(uint8_t palleteIndex, uint8_t emphasis);
    uint32_t* GetScanlinePtr();

    void HBlank();
    void VBlank();

    const uint32_t* Buffer() const;

#ifdef DIAGNOSTIC
    static const int WIDTH{ 341 };
    static const int HEIGHT{ 261 };
#else
    static const int WIDTH{ 256 };
    static const int HEIGHT{ 240 };
#endif

private:
    std::array<uint32_t, WIDTH * HEIGHT> buffer_;
    uint32_t* currentPixelAddress_;

    static std::array<std::array<uint32_t, 64>, 8> Palette;
};