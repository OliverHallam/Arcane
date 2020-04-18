#include "pch.h"

#include "WasapiRenderer.h"


void WasapiRenderer::Initialize()
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    auto deviceEnumerator = winrt::create_instance<IMMDeviceEnumerator>(__uuidof(MMDeviceEnumerator));

    auto hr = deviceEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eMultimedia, endpoint_.put());
    winrt::check_hresult(hr);


}
