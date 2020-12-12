#pragma once

#include <cstdint>

class QpcTimer
{
public:
    static uint64_t Frequency();
    static uint64_t Current();
};