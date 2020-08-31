#pragma once

#include <cstdint>

enum class SyncEvent : uint32_t
{
    None,
    ApuSample,
    ApuSync,
};