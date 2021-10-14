#include "pch.h"

#include "DefaultShaders.h"

#include "Shaders\DefaultVertexShader.h"
#include "Shaders\DefaultPixelShader.h"

void DefaultShaders::Create(ID3D11Device* device)
{
    CreateShaders(device);
    CreateInputLayout(device);
    CreateSamplerState(device);
}

void DefaultShaders::PrepareRenderState(ID3D11DeviceContext* context) const
{
    context->IASetInputLayout(inputLayout_.get());

    context->VSSetShader(vertexShader_.get(), nullptr, 0);

    context->PSSetShader(pixelShader_.get(), nullptr, 0);

    ID3D11SamplerState* const samplers[1] = { samplerState_.get() };
    context->PSSetSamplers(0, 1, samplers);
}

void DefaultShaders::CreateShaders(ID3D11Device* device)
{
    auto hr = device->CreateVertexShader(
        DefaultVertexShaderBytecode,
        _countof(DefaultVertexShaderBytecode),
        nullptr,
        vertexShader_.put());
    winrt::check_hresult(hr);

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

void DefaultShaders::CreateSamplerState(ID3D11Device* device)
{
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    auto hr = device->CreateSamplerState(&samplerDesc, samplerState_.put());
    winrt::check_hresult(hr);
}
