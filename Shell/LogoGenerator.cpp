#include "pch.h"

#include "LogoGenerator.h"

enum StrokeShape : uint8_t
{
    STROKE_SHAPE_RECT,
    STROKE_SHAPE_CORNER,
    STROKE_SHAPE_CURVE,

    STROKE_START_CAP = 0x10,
    STROKE_END_CAP = 0x20
};

struct Stroke
{
    uint16_t startX;
    uint16_t startY;

    uint8_t shape;
    uint8_t orientation; // up, right, down, left
    uint16_t length;

    float startTime;
    float duration;
};

const int LineWidth = 10;

Stroke Strokes[] {
    // A
    { 10, 5, STROKE_SHAPE_RECT | STROKE_START_CAP, 0, 40, 0, 4 },
    { 15, 45, STROKE_SHAPE_CORNER, 0, 5, 4, 1 },
    { 15, 50, STROKE_SHAPE_RECT, 1, 40, 5, 4 },
    { 15, 30, STROKE_SHAPE_RECT, 1, 40, 7, 4 },
    { 55, 45, STROKE_SHAPE_CORNER, 1, 5, 9, 1 },
    { 60, 45, STROKE_SHAPE_RECT | STROKE_END_CAP, 2, 40, 10, 4 },

    // R
    { 75, 5, STROKE_SHAPE_RECT | STROKE_START_CAP | STROKE_END_CAP, 0, 50, 8, 5},
    { 80, 50, STROKE_SHAPE_RECT, 1, 35, 13, 3.5},
    { 80, 30, STROKE_SHAPE_RECT, 1, 35, 15, 3.5 },
    { 115, 50, STROKE_SHAPE_CURVE, 1, 10, 16.5, 2 },
    { 125, 40, STROKE_SHAPE_CURVE, 2, 10, 20, 2 },
    { 115, 30, STROKE_SHAPE_CURVE, 1, 10, 18.5, 2 },
    { 125, 20, STROKE_SHAPE_RECT | STROKE_END_CAP, 2, 15, 20.5, 1.5 },

    // C
    { 185, 10, STROKE_SHAPE_RECT | STROKE_START_CAP, 3, 40, 16, 4},
    { 145, 15, STROKE_SHAPE_CORNER, 3, 5, 20, 1 },
    { 140, 15, STROKE_SHAPE_RECT, 0, 30, 21, 3},
    { 145, 45, STROKE_SHAPE_CORNER, 0, 5, 24, 1 },
    { 145, 50, STROKE_SHAPE_RECT | STROKE_END_CAP, 1, 40, 25, 4 },

    // A
    { 195, 5, STROKE_SHAPE_RECT | STROKE_START_CAP, 0, 40, 23, 4 },
    { 200, 45, STROKE_SHAPE_CORNER, 0, 5, 27, 1 },
    { 200, 50, STROKE_SHAPE_RECT, 1, 40, 28, 4 },
    { 200, 30, STROKE_SHAPE_RECT, 1, 40, 30, 4 },
    { 240, 45, STROKE_SHAPE_CORNER, 1, 5, 32, 1 },
    { 245, 45, STROKE_SHAPE_RECT | STROKE_END_CAP, 2, 40, 33, 4 },

    // N
    { 260, 5, STROKE_SHAPE_RECT | STROKE_START_CAP, 0, 40, 31, 4 },
    { 265, 45, STROKE_SHAPE_CORNER, 0, 5, 35, 1 },
    { 265, 50, STROKE_SHAPE_RECT, 1, 40, 36, 4 },
    { 305, 45, STROKE_SHAPE_CORNER, 1, 5, 40, 1 },
    { 310, 45, STROKE_SHAPE_RECT | STROKE_END_CAP, 2, 40, 41, 4 },

    // E
    { 325, 5, STROKE_SHAPE_RECT | STROKE_START_CAP, 0, 40, 39, 4 },
    { 330, 45, STROKE_SHAPE_CORNER, 0, 5, 43, 1 },
    { 330, 50, STROKE_SHAPE_RECT | STROKE_END_CAP, 1, 50, 44, 5 },
    { 330, 30, STROKE_SHAPE_RECT | STROKE_END_CAP, 1, 50, 46, 5 },
    { 330, 10, STROKE_SHAPE_RECT | STROKE_END_CAP, 1, 50, 48, 5 },
};

#define PI 3.1415926535

LogoGenerator::LogoGenerator()
{
    for (auto i = 0; i <= SUBDIVISION_COUNT; i++)
    {
        sin_[i] = sin(PI / 2 * i / SUBDIVISION_COUNT);
        cos_[i] = cos(PI / 2 * i / SUBDIVISION_COUNT);
    }
}

int LogoGenerator::MaxVertexCount() const
{
    int count = 0;
    for (auto& stroke : Strokes)
    {
        switch (stroke.shape & 0x0f)
        {
        case STROKE_SHAPE_RECT:
            count += 4;
            break;
        case STROKE_SHAPE_CORNER:
            count += SUBDIVISION_COUNT + 2;
            break;
        case STROKE_SHAPE_CURVE:
            count += 2 * SUBDIVISION_COUNT + 2;
            break;
        }
    }

    return count * 4;
}

int LogoGenerator::MaxIndexCount() const
{
    int count = 0;
    for (auto & stroke: Strokes)
    {
        switch (stroke.shape & 0x0f)
        {
        case STROKE_SHAPE_RECT:
            count += 6;
            break;
        case STROKE_SHAPE_CORNER:
            count += SUBDIVISION_COUNT *3;
            break;
        case STROKE_SHAPE_CURVE:
            count += 6 * SUBDIVISION_COUNT;
            break;
        }
    }

    return count * 2;
}

float LogoGenerator::MaxTime() const
{
    return 5.3;
}


int LogoGenerator::FillBuffers(float time, Vertex* vertices, uint16_t* indices) const
{
    uint16_t index = 0;
    auto firstIndex = indices;
    FillBuffers(time, vertices, indices, index, 0.5, 0.5, 0.5, 2);
    FillBuffers(time, vertices, indices, index, 0.75, 0, 0, 0);
    return static_cast<int>(indices - firstIndex);
}

void LogoGenerator::FillBuffers(float time, Vertex*& vertices, uint16_t*& indices, uint16_t& index, float r, float g, float b, float dilation) const
{
    auto lineWidth = LineWidth + 2 * dilation;

    time *= 10;


    for (auto& stroke : Strokes)
    {
        float left=0, right=0, top=0, bottom=0;

        float t = (time - stroke.startTime) / stroke.duration;
        if (t > 1)
            t = 1;
        else if (t < 0)
            break;

        switch (stroke.shape & 0x0f)
        {
        case STROKE_SHAPE_RECT:
        {
            switch (stroke.orientation)
            {
            case 0:
                left = stroke.startX - lineWidth / 2;
                right = stroke.startX + lineWidth / 2;
                bottom = stroke.startY;
                top = stroke.startY + t * stroke.length;

                if (stroke.shape & STROKE_START_CAP)
                    bottom -= dilation;
                if (stroke.shape & STROKE_END_CAP)
                    top += dilation;
                break;

            case 1:
                left = stroke.startX;
                right = stroke.startX + t * stroke.length;
                bottom = stroke.startY - lineWidth / 2;
                top = stroke.startY + lineWidth / 2;

                if (stroke.shape & STROKE_START_CAP)
                    left -= dilation;
                if (stroke.shape & STROKE_END_CAP)
                    right += dilation;
                break;

            case 2:
                left = stroke.startX - lineWidth / 2;
                right = stroke.startX + lineWidth / 2;
                bottom = stroke.startY - t * stroke.length;
                top = stroke.startY;

                if (stroke.shape & STROKE_START_CAP)
                    top += dilation;
                if (stroke.shape & STROKE_END_CAP)
                    bottom -= dilation;
                break;

            case 3:
                left = stroke.startX - t * stroke.length;
                right = stroke.startX;
                bottom = stroke.startY - lineWidth / 2;
                top = stroke.startY + lineWidth / 2;

                if (stroke.shape & STROKE_START_CAP)
                    right += dilation;
                if (stroke.shape & STROKE_END_CAP)
                    left -= dilation;

                break;
            }

            *vertices++ = { left, top, r, g, b, 1 };
            *vertices++ = { right, top, r, g, b, 1 };
            *vertices++ = { left, bottom, r, g, b, 1 };
            *vertices++ = { right, bottom, r, g, b, 1 };

            *indices++ = index + 0;
            *indices++ = index + 1;
            *indices++ = index + 2;
            *indices++ = index + 1;
            *indices++ = index + 3;
            *indices++ = index + 2;
            index += 4;
            break;
        }

        case STROKE_SHAPE_CORNER:
        {
            auto divisions = static_cast<int>((SUBDIVISION_COUNT)*t);

            float centerX = stroke.startX;
            float centerY = stroke.startY;
            *vertices++ = { centerX, centerY, r, g, b, 1 };

            auto radius = LineWidth + dilation;

            switch (stroke.orientation)
            {
            case 0:
            {
                for (auto i = 0; i <= divisions; i++)
                {
                    *vertices++ = { centerX - cos_[i] * radius, centerY + sin_[i] * radius, r, g, b, 1 };
                }
                break;
            }

            case 1:
            {
                for (auto i = 0; i <= divisions; i++)
                {
                    *vertices++ = { centerX + sin_[i] * radius, centerY + cos_[i] * radius, r, g, b, 1 };
                }
                break;
            }

            case 2:
            {
                for (auto i = 0; i <= divisions; i++)
                {
                    *vertices++ = { centerX + cos_[i] * radius, centerY - sin_[i] * radius, r, g, b, 1 };
                }
                break;
            }

            case 3:
            {
                for (auto i = 0; i <= divisions; i++)
                {
                    *vertices++ = { centerX - sin_[i] * radius, centerY - cos_[i] * radius, r, g, b, 1 };
                }
                break;
            }
            }

            for (auto i = 0; i < divisions; i++)
            {
                *indices++ = index + 0;
                *indices++ = index + i + 1;
                *indices++ = index + i + 2;
            }

            index += divisions + 2;
            break;
        }

        case STROKE_SHAPE_CURVE:
        {
            auto divisions = static_cast<int>((SUBDIVISION_COUNT)*t);

            float innerRadius = stroke.length - lineWidth / 2;
            float outerRadius = stroke.length + lineWidth / 2;
            switch (stroke.orientation)
            {
            case 0:
            {
                float centerX = stroke.startX + stroke.length;
                float centerY = stroke.startY;

                for (auto i = 0; i <= divisions; i++)
                {
                    *vertices++ = { centerX - cos_[i] * innerRadius, centerY + sin_[i] * innerRadius, r, g, b, 1 };
                    *vertices++ = { centerX - cos_[i] * outerRadius, centerY + sin_[i] * outerRadius, r, g, b, 1 };
                }
                break;
            }

            case 1:
            {
                float centerX = stroke.startX;
                float centerY = stroke.startY - stroke.length;

                for (auto i = 0; i <= divisions; i++)
                {
                    *vertices++ = { centerX + sin_[i] * innerRadius, centerY + cos_[i] * innerRadius, r, g, b, 1 };
                    *vertices++ = { centerX + sin_[i] * outerRadius, centerY + cos_[i] * outerRadius, r, g, b, 1 };
                }
                break;
            }

            case 2:
            {
                float centerX = stroke.startX - stroke.length;
                float centerY = stroke.startY;

                for (auto i = 0; i <= divisions; i++)
                {
                    *vertices++ = { centerX + cos_[i] * innerRadius, centerY - sin_[i] * innerRadius, r, g, b, 1 };
                    *vertices++ = { centerX + cos_[i] * outerRadius, centerY - sin_[i] * outerRadius, r, g, b, 1 };
                }
                break;
            }

            case 3:
            {
                float centerX = stroke.startX - stroke.length;
                float centerY = stroke.startY;

                for (auto i = 0; i <= divisions; i++)
                {
                    *vertices++ = { centerX - sin_[i] * innerRadius, centerY - cos_[i] * innerRadius, r, g, b, 1 };
                    *vertices++ = { centerX - sin_[i] * outerRadius, centerY - cos_[i] * outerRadius, r, g, b, 1 };
                }
                break;
            }
            }


            for (auto i = 0; i < divisions; i++)
            {
                *indices++ = index + 0;
                *indices++ = index + 1;
                *indices++ = index + 2;
                *indices++ = index + 1;
                *indices++ = index + 3;
                *indices++ = index + 2;
                index += 2;
            }

            index += 2;
        }
        }
    }

}
