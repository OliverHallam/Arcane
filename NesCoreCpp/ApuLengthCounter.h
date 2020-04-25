#pragma once

#include <cstdint>

class ApuLengthCounter
{
public:
    void Tick();

    void SetHalt(bool halt);
    void SetLength(uint8_t length);
    void SetEnabled(bool enabled);

    bool IsEnabled() const;

private:
    uint8_t GetLinearLength(uint8_t length);

    bool enabled_{};
    bool halt_{};
    uint8_t length_{};
};