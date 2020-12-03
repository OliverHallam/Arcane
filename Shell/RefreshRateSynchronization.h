#pragma once

#include <cstdint>

class RefreshRateSynchronization
{
public:
    RefreshRateSynchronization();

    void Reset(uint32_t simulationFrequency, uint32_t displayFrequency);

    // the number of frames to emulate for the current frame
    uint32_t SimulatedFrames() const;

    // the number of vsyncs to display the current frame
    uint32_t DisplayFrames() const;

    // returns true if the current emulated frame exactly lines up with the current displayed frame
    bool IsSynchronized() const;

    void NextFrame();

private:
    uint32_t GCD(uint32_t x, uint32_t y) const;

    uint32_t fractionalFrames_;

    uint32_t simulatedFrames_;
    uint32_t displayFrames_;

    uint32_t fractionalFramesPerSimulatedFrame_;
    uint32_t divisor_;
};