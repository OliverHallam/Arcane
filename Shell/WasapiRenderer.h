#pragma once

#include <mmdeviceapi.h>

class WasapiRenderer
{
public:
    void Initialize();

private:
    winrt::com_ptr<IMMDevice> endpoint_;
};