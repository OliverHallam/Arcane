#pragma once

#include <cstdint>

static class QpcTimer
{
public:
    static uint64_t Frequency();
    static uint64_t Current();
};