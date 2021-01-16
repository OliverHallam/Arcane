#include "ApuDmc.h"

#include "Bus.h"

#include <cassert>

ApuDmc::ApuDmc(Apu& apu)
    : apu_{ apu }
{
}

void ApuDmc::Enable(bool enabled)
{
    if (enabled)
    {
        if (state_.sampleBytesRemaining_ == 0)
        {
            state_.currentAddress_ = state_.sampleAddress_;
            if (state_.sampleLength_)
            {
                state_.sampleBytesRemaining_ = state_.sampleLength_;

                if (!state_.sampleBufferHasData_)
                    RequestByte();

                apu_.ScheduleDmc(state_.timer_ + (7 - state_.sampleShift_) * state_.rate_);
            }
        }
    }
    else
    {
        apu_.ClearDmcByteRequest();
        state_.byteRequested_ = false;
        state_.sampleBytesRemaining_ = 0;
    }

    apu_.SetDmcInterrupt(false);
}

bool ApuDmc::IsEnabled() const
{
    return state_.sampleBytesRemaining_;
}

void ApuDmc::Write(uint16_t address, uint8_t value)
{
    switch (address & 0x0003)
    {
    case 0:
    {
        auto prevIrqEnabled = state_.irqEnabled_;
        auto prevLoop = state_.loop_;
        state_.irqEnabled_ = value & 0x80;
        state_.loop_ = value & 0x40;
        state_.rate_ = GetLinearRate(value & 0x0f);

        if (!state_.irqEnabled_)
            apu_.SetDmcInterrupt(false);

        // TODO: work out  when we actually need to do this.
        apu_.ScheduleDmc(state_.timer_ + (7 - state_.sampleShift_) * state_.rate_);

        break;
    }

    case 1:
        // TODO: this can be missed if a clock is happening at the same time
        state_.level_ = value & 0x7f;
        break;

    case 2:
        state_.currentAddress_ = state_.sampleAddress_ = 0xc000 | (value << 6);
        break;

    case 3:
    {
        bool prevSampleLength = state_.sampleLength_;
        state_.sampleLength_ = (value << 4) + 1;

        if (state_.sampleLength_ && !prevSampleLength && state_.loop_ && state_.sampleBytesRemaining_ <= 1);
            apu_.ScheduleDmc(state_.timer_ + (7 - state_.sampleShift_) * state_.rate_);
        break;
    }
    }
}

void ApuDmc::Run(uint32_t cycles)
{
    state_.timer_ -= cycles;

    while (state_.timer_ <= 0)
    {
        Clock();

        state_.timer_ += state_.rate_;

        if (!state_.sampleBufferHasData_ && state_.sampleBytesRemaining_ > 0)
        {
            assert(state_.timer_ == state_.rate_);

            RequestByte();
        }
    }
}

void ApuDmc::SetBuffer(uint8_t value)
{
    assert(state_.sampleBytesRemaining_);

    state_.sampleBuffer_ = value;
    state_.sampleBufferHasData_ = true;
    state_.byteRequested_ = false;

    state_.currentAddress_++;
    state_.currentAddress_ |= 0x8000;
    state_.sampleBytesRemaining_--;

    if (state_.sampleBytesRemaining_ == 0)
    {
        if (!state_.loop_)
        {
            state_.outBuffer_ = 0;
            state_.outBufferHasData_ = false;
            if (state_.irqEnabled_)
                apu_.SetDmcInterrupt(true);
            return;
        }

        state_.sampleBytesRemaining_ = state_.sampleLength_;
        state_.currentAddress_ = state_.sampleAddress_;
    }
}

uint8_t ApuDmc::Sample() const
{
    return state_.level_;
}

void ApuDmc::CaptureState(ApuDmcState* state) const
{
    *state = state_;
}

void ApuDmc::RestoreState(const ApuDmcState& state)
{
    state_ = state;
}

void ApuDmc::Clock()
{
    if (state_.outBufferHasData_)
    {
        auto nextBit = (state_.outBuffer_ >> state_.sampleShift_) & 1;

        if (nextBit)
        {
            if (state_.level_ <= 125)
                state_.level_ += 2;
        }
        else
        {
            if (state_.level_ >= 2)
                state_.level_ -= 2;
        }
    }

    if (state_.sampleShift_ == 7)
    {
        state_.sampleShift_ = 0;
        state_.outBuffer_ = state_.sampleBuffer_;
        state_.outBufferHasData_ = state_.sampleBufferHasData_;
        state_.sampleBufferHasData_ = false;

        if (state_.sampleBytesRemaining_ > 1 || (state_.loop_ && state_.sampleLength_) || state_.irqEnabled_)
        {
            apu_.ScheduleDmc(state_.timer_ + 8 * state_.rate_);
        }
    }
    else
        state_.sampleShift_++;
}

void ApuDmc::RequestByte()
{
    if (state_.byteRequested_)
        return;

    assert(state_.sampleBytesRemaining_);

    state_.byteRequested_ = true;
    apu_.RequestDmcByte(state_.currentAddress_);
}

uint32_t ApuDmc::GetLinearRate(uint8_t rate)
{
    switch (rate)
    {
    case 0: return 428;
    case 1: return 380;
    case 2: return 340;
    case 3: return 320;
    case 4: return 286;
    case 5: return 254;
    case 6: return 226;
    case 7: return 214;
    case 8: return 190;
    case 9: return 160;
    case 10: return 142;
    case 11: return 128;
    case 12: return 106;
    case 13: return 84;
    case 14: return 72;
    case 15: return 54;
    }

    return 0;
}
