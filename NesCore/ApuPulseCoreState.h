#pragma once

struct ApuPulseCoreState
{
    int32_t timer_{};
    uint32_t sequence_{};
    uint32_t dutyLookup_{};
};