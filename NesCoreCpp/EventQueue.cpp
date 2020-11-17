#include "EventQueue.h"

#include <algorithm>

EventQueue::EventQueue()
    : count_(0)
{
}

void EventQueue::Schedule(uint32_t cycles, SyncEvent value)
{
    heap_[count_++] = Event(cycles, value);
    std::push_heap(begin(heap_), begin(heap_) + count_);
}

void EventQueue::Unschedule(SyncEvent value)
{
    auto current = begin(heap_);
    auto end = current + count_;
    for (auto current = begin(heap_); current != end; ++current)
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
    return count_ == 0;
}

uint32_t EventQueue::GetNextEventTime() const
{
    return begin(heap_)->Cycles;
}

SyncEvent EventQueue::PopEvent()
{
    auto value = begin(heap_)->Value;
    std::pop_heap(begin(heap_), begin(heap_) + count_);
    count_--;
    return value;
}

EventQueue::Event::Event(uint32_t cycles, SyncEvent value)
    : Cycles { cycles },
    Value { value }
{
}

uint64_t EventQueue::Event::GetCompareValue()
{
    return (static_cast<uint64_t>(Cycles) << 32) | static_cast<uint32_t>(Value);
}

bool EventQueue::Event::operator<(Event other)
{
    auto thisValue = GetCompareValue();
    auto otherValue = other.GetCompareValue();
    auto cmp = static_cast<int64_t>(thisValue - otherValue);
    return cmp > 0;
}
