#pragma once

#include "SyncEvent.h"

#include <cstdint>
#include <array>

class EventQueue
{
public:
    EventQueue();

    static const uint32_t MAX_EVENTS = 32;

    void Schedule(uint32_t cycles, SyncEvent value);
    void Unschedule(SyncEvent value);

    bool Empty() const;
    uint32_t GetNextEventTime() const;
    SyncEvent PopEvent();

private:
    struct Event
    {
    public:
        Event() = default;
        Event(uint32_t cycles, SyncEvent value);

        uint32_t Cycles;
        SyncEvent Value;

        uint64_t GetCompareValue();

        bool operator<(Event other);
    };


    uint32_t count_;
    std::array<Event, MAX_EVENTS> heap_;
};