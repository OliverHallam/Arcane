#include "pch.h"
#include "RewindBuffer.h"

RewindBuffer::RewindBuffer()
    : currentIndex_{ 0 },
    firstIndex_{ 0 }
{
}

void RewindBuffer::Clear()
{
    firstIndex_ = 0;
    currentIndex_ = 0;
}

SystemState* RewindBuffer::Push()
{
    auto state = &buffer_[currentIndex_];

    currentIndex_++;
    if (currentIndex_ == BUFFER_SIZE)
        currentIndex_ = 0;

    if (currentIndex_ == firstIndex_)
    {
        firstIndex_++;
        if (firstIndex_ == BUFFER_SIZE)
            firstIndex_ = 0;
    }

    return state;
}

SystemState* RewindBuffer::Pop()
{
    if (currentIndex_ == firstIndex_)
        return nullptr;

    if (currentIndex_ == 0)
        currentIndex_ = BUFFER_SIZE - 1;
    else
        currentIndex_--;

    return &buffer_[currentIndex_];
}
