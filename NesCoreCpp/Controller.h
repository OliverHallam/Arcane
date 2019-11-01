#pragma once

#include <cstdint>

class Controller
{
public:
    void Up(bool pressed);
    void Down(bool pressed);
    void Left(bool pressed);
    void Right(bool pressed);

    void A(bool pressed);
    void B(bool pressed);

    void Select(bool pressed);
    void Start(bool pressed);

    uint8_t Read();
    void Write(uint8_t data);

private:
    void CaptureState();

    bool up_, down_, left_, right_;
    bool a_, b_;
    bool select_, start_;

    bool strobe_;
    uint8_t noise_;
    uint8_t state_;
};