#pragma once

#include <cstdint>

class ApuSweep
{
public:
    void SetPeriodHigh(uint8_t value);
    void SetPeriodLow(uint8_t value);
    void SetSweep(uint8_t value);

    void Tick();

    uint16_t Period();

    bool IsOutputEnabled();

private:
    void UpdateTargetPeriod();

    uint16_t period_{};

    bool enabled_{};
    uint_fast8_t divide_{};
    bool negate_{};
    uint_fast8_t shift_{};
    bool reload_{};
    uint_fast8_t divideCounter_{};
    uint16_t targetPeriod_{};
};