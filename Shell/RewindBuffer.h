#pragma once

#include "../NesCore/SystemState.h"

class RewindBuffer
{
public:
    RewindBuffer();

    void Clear();

    SystemState* Push();
    SystemState* Pop();

private:
    static const int BUFFER_SIZE = 3600;

    int currentIndex_;
    int firstIndex_;
    std::array<SystemState, BUFFER_SIZE> buffer_;
};