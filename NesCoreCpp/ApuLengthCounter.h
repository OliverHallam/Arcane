#pragma once

#include <cstdint>

class ApuLengthCounter
{
public:
    void Tick();
    bool IsOutputEnabled();

    void SetHalt(bool halt);
    void SetEnabled(bool enabled);
    void SetLength(uint8_t length);

private:
    uint8_t GetLinearLength(uint8_t length);

    bool enabled_{};
    bool halt_{};
    uint8_t length_{};

    bool outputEnabled_{};
};