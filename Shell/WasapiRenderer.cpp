#include "pch.h"

#include "WasapiRenderer.h"

#include <mmdeviceapi.h>

void WasapiRenderer::Initialize()
{
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);


}
