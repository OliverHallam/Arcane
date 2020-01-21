#pragma once

#include <array>
#include <cstdint>

class Display
{
public:
    Display();

    uint32_t GetPixel(uint8_t palleteIndex);
    void WritePixel(uint32_t pixel);

    void HBlank();
    void VBlank();

    const uint32_t* Buffer() const;

    static const int WIDTH{ 256 };
    static const int HEIGHT{ 240 };

private:
    std::array<uint32_t, WIDTH * HEIGHT> buffer_;
    uint32_t* currentPixelAddress_;

    static std::array<uint32_t, 64> Pallette;
};