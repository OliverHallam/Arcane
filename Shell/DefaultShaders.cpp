#include "pch.h"

#include "DefaultShaders.h"

#include "Shaders\DefaultVertexShader.h"
#include "Shaders\DefaultPixelShader.h"

void DefaultShaders::Create(ID3D11Device* device)
{
    CreateShaders(device);
    CreateInputLayout(device);
}

void DefaultShaders::PrepareRenderState(ID3D11DeviceContext* context) const
{
    context->IASetInputLayout(inputLayout_.get());

    context->VSSetShader(vertexShader_.get(), nullptr, 0);

    context->PSSetShader(pixelShader_.get(), nullptr, 0);
}

void DefaultShaders::CreateShaders(ID3D11Device* device)
{
    auto hr = device->CreateVertexShader(
        DefaultVertexShaderBytecode,
        _countof(DefaultVertexShaderBytecode),
        nullptr,
        vertexShader_.put());
    winrt::check_hresult(hr);

    winrt::com_ptr<ID3D11PixelShader> pixelShader;
    hr = device->CreatePixelShader(
        DefaultPixelShaderBytecode,
        _countof(DefaultPixelShaderBytecode),
        nullptr,
        pixelShader_.put());
    winrt::check_hresult(hr);
}

void DefaultShaders::CreateInputLayout(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    auto hr = device->CreateInputLayout(
        vertexLayoutDesc,
        _countof(vertexLayoutDesc),
        DefaultVertexShaderBytecode,
        _countof(DefaultVertexShaderBytecode),
        inputLayout_.put());
    winrt::check_hresult(hr);
}
