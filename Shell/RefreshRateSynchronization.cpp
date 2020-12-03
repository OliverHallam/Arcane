#include "pch.h"
#include "RefreshRateSynchronization.h"

RefreshRateSynchronization::RefreshRateSynchronization() :
    fractionalFrames_{ 0 },
    simulatedFrames_{ 1 },
    displayFrames_{ 1 },
    fractionalFramesPerSimulatedFrame_{ 1 },
    divisor_{ 1 }
{
}

void RefreshRateSynchronization::Reset(uint32_t simulationFrequency, uint32_t displayFrequency)
{
    auto gcd = GCD(simulationFrequency, displayFrequency);

    simulationFrequency /= gcd;
    displayFrequency /= gcd;

    fractionalFramesPerSimulatedFrame_ = displayFrequency;
    divisor_ = simulationFrequency;

    fractionalFrames_ = 0;
}

uint32_t RefreshRateSynchronization::SimulatedFrames() const
{
    return simulatedFrames_;
}

uint32_t RefreshRateSynchronization::DisplayFrames() const
{
    return displayFrames_;
}

bool RefreshRateSynchronization::IsSynchronized() const
{
    return fractionalFrames_ == 0;
}

void RefreshRateSynchronization::NextFrame()
{
    simulatedFrames_ = 0;
    displayFrames_ = 0;

    do
    {
        fractionalFrames_ += fractionalFramesPerSimulatedFrame_;
        while (fractionalFrames_ >= divisor_)
        {
            fractionalFrames_ -= divisor_;
            displayFrames_++;
        }

        simulatedFrames_++;
    } while (displayFrames_ == 0);
}

uint32_t RefreshRateSynchronization::GCD(uint32_t x, uint32_t y) const
{
    while (x != 0)
    {
        auto tmp = x;
        x = y % x;
        y = tmp;
    }

    return y;
}
