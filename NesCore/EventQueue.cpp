#include "EventQueue.h"

#include <algorithm>

EventQueue::EventQueue()
    : count_(0),
    heap_{}
{
}

void EventQueue::Schedule(uint32_t cycles, SyncEvent value)
{
    if (count_ >= MAX_EVENTS)
        Tidy();

    heap_[count_++] = Event(cycles, value);
    std::push_heap(begin(heap_), begin(heap_) + count_);
}

bool EventQueue::Deschedule(SyncEvent value)
{
    auto current = begin(heap_);
    auto end = current + count_;
    for (auto current = begin(heap_); current != end; ++current)
    {
        if (current->Value == value)
        {
            current->Value = SyncEvent::None;
            return true;
        }
    }

    return false;
}

bool EventQueue::DescheduleAll(SyncEvent value)
{
    bool descheduled = false;

    auto current = begin(heap_);
    auto end = current + count_;
    for (auto current = begin(heap_); current != end; ++current)
    {
        if (current->Value == value)
        {
            current->Value = SyncEvent::None;
            descheduled = true;
        }
    }

    return descheduled;
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

void EventQueue::Tidy()
{
    int count = 0;
    for (auto i = 0; i < MAX_EVENTS; i++)
    {
        if (heap_[i].Value != SyncEvent::None)
            heap_[count++] = heap_[i];
    }

    count_ = count;
    std::make_heap(begin(heap_), begin(heap_) + count_);
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
