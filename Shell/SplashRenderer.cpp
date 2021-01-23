#include "pch.h"

#include "SplashRenderer.h"

#include "Shaders\SplashVertexShader.h"
#include "Shaders\SplashPixelShader.h"

void SplashRenderer::Create(ID3D11Device* device)
{
    CreateShaders(device);
    CreateVertexBuffer(device);
    CreateIndexBuffer(device);
    CreateInputLayout(device);
}

void SplashRenderer::PrepareRenderState(ID3D11DeviceContext* context) const
{
    context->IASetInputLayout(inputLayout_.get());
    context->VSSetShader(vertexShader_.get(), nullptr, 0);
    context->PSSetShader(pixelShader_.get(), nullptr, 0);

    UINT stride = sizeof(LogoGenerator::Vertex);
    UINT offset = 0;
    ID3D11Buffer* const vertexBuffers[1] = { vertexBuffer_.get() };
    context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer_.get(), DXGI_FORMAT_R16_UINT, 0);
}

bool SplashRenderer::PrepareRender(ID3D11DeviceContext* context, float time)
{
    D3D11_MAPPED_SUBRESOURCE vertexResource;
    auto hr = context->Map(vertexBuffer_.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &vertexResource);
    winrt::check_hresult(hr);

    D3D11_MAPPED_SUBRESOURCE indexResource;
    hr = context->Map(indexBuffer_.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &indexResource);
    winrt::check_hresult(hr);

    indexCount_ = generator_.FillBuffers(
        time,
        reinterpret_cast<LogoGenerator::Vertex*>(vertexResource.pData),
        reinterpret_cast<uint16_t*>(indexResource.pData));

    context->Unmap(vertexBuffer_.get(), 0);
    context->Unmap(indexBuffer_.get(), 0);

    return time <= generator_.MaxTime();
}

void SplashRenderer::Render(ID3D11DeviceContext* context)
{
    context->DrawIndexed(indexCount_, 0, 0);
}

void SplashRenderer::CreateVertexBuffer(ID3D11Device* device)
{
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = generator_.MaxVertexCount() * sizeof(LogoGenerator::Vertex);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    auto hr = device->CreateBuffer(&bufferDesc, NULL, vertexBuffer_.put());
    winrt::check_hresult(hr);
}

void SplashRenderer::CreateIndexBuffer(ID3D11Device* device)
{
    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = generator_.MaxIndexCount() * sizeof(uint16_t);
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    auto hr = device->CreateBuffer(&bufferDesc, NULL, indexBuffer_.put());
    winrt::check_hresult(hr);
}

void SplashRenderer::CreateShaders(ID3D11Device* device)
{
    auto hr = device->CreateVertexShader(
        SplashVertexShaderBytecode,
        _countof(SplashVertexShaderBytecode),
        nullptr,
        vertexShader_.put());
    winrt::check_hresult(hr);

    winrt::com_ptr<ID3D11PixelShader> pixelShader;
    hr = device->CreatePixelShader(
        SplashPixelShaderBytecode,
        _countof(SplashPixelShaderBytecode),
        nullptr,
        pixelShader_.put());
    winrt::check_hresult(hr);
}

void SplashRenderer::CreateInputLayout(ID3D11Device* device)
{
    D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    auto hr = device->CreateInputLayout(
        vertexLayoutDesc,
        _countof(vertexLayoutDesc),
        SplashVertexShaderBytecode,
        _countof(SplashVertexShaderBytecode),
        inputLayout_.put());
    winrt::check_hresult(hr);
}
