#include "EventQueue.h"

#include <algorithm>

EventQueue::EventQueue()
    : end_(begin(heap_))
{
}

void EventQueue::Schedule(uint32_t cycles, SyncEvent value)
{
    *end_++ = Event(cycles, value);
    std::push_heap(begin(heap_), end_);
}

void EventQueue::Unschedule(SyncEvent value)
{
    auto current = begin(heap_);
    for (auto current = begin(heap_); current != end_; ++current)
    {
        if (current->Value == value)
        {
            current->Value = SyncEvent::None;
            return;
        }
    }
}

bool EventQueue::Empty() const
{
    return begin(heap_) == end_;
}

uint32_t EventQueue::GetNextEventTime() const
{
    return begin(heap_)->Cycles;
}

SyncEvent EventQueue::PopEvent()
{
    auto value = begin(heap_)->Value;
    std::pop_heap(begin(heap_), end_--);
    return value;
}

EventQueue::Event::Event(uint32_t cycles, SyncEvent value)
    : Cycles { cycles },
    Value { value }
{
}

bool EventQueue::Event::operator<(Event other)
{
    // this should handle integer wraparound properly
    auto cmp = static_cast<int>(Cycles - other.Cycles);
    return cmp > 0;
}
