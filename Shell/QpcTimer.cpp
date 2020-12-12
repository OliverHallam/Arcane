#include "pch.h"

#include "QpcTimer.h"

uint64_t QpcTimer::Frequency()
{
    LARGE_INTEGER largeInteger;
    auto success = QueryPerformanceFrequency(&largeInteger);
    assert(success); // shouldn't fail after windows XP
    return largeInteger.QuadPart;
}

uint64_t QpcTimer::Current()
{
    LARGE_INTEGER largeInteger;
    auto success = QueryPerformanceCounter(&largeInteger);
    assert(success); // shouldn't fail after windows XP
    return largeInteger.QuadPart;
}
