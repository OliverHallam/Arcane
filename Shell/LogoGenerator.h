#pragma once

#include <cstdint>


class LogoGenerator
{
public:
    LogoGenerator();

    struct Vertex
    {
        float x;
        float y;

        float r;
        float g;
        float b;
        float a;
    };

    int MaxVertexCount() const;
    int MaxIndexCount() const;
    float MaxTime() const;

    int FillBuffers(float time, Vertex* vertices, uint16_t* indices) const;

    static const int SUBDIVISION_COUNT = 8;

private:
    void FillBuffers(float time, Vertex*& vertices, uint16_t*& indices, uint16_t& index, float r, float g, float b, float dilation) const;

    std::array<float, SUBDIVISION_COUNT + 1> sin_;
    std::array<float, SUBDIVISION_COUNT + 1> cos_;
};