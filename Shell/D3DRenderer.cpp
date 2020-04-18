#include "pch.h"

#include "D3DRenderer.h"

#include "VertexShader.h"
#include "PixelShader.h"

D3DRenderer::D3DRenderer(uint32_t width, uint32_t height)
    : width_{width},
    height_{height}
{
}

bool D3DRenderer::Initialize(HWND window)
{
    return CreateDevice()
        && CreateSwapChain(window)
        && CreateRenderTarget()
        && CreateVertexBuffer()
        && CreateIndexBuffer()
        && CreateFrameBuffer()
        && CreateShaders()
        && CreateInputLayout()
        && CreateShaderParameters()
        && CreateRasterizerState();
}


void D3DRenderer::PrepareRenderState()
{
    // Input assembler
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11Buffer* const vertexBuffers[1] = { vertexBuffer_.get() };
    deviceContext_->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
    deviceContext_->IASetInputLayout(inputLayout_.get());
    deviceContext_->IASetIndexBuffer(indexBuffer_.get(), DXGI_FORMAT_R16_UINT, 0);
    deviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // vertex shader
    deviceContext_->VSSetShader(vertexShader_.get(), nullptr, 0);

    // rasterizer
    deviceContext_->RSSetState(rasterizerState_.get());

    // pixel shader
    deviceContext_->PSSetShader(pixelShader_.get(), nullptr, 0);
    ID3D11ShaderResourceView* const shaderResourceViews[1] = { frameBufferShaderResourceView_.get() };
    deviceContext_->PSSetShaderResources(0, 1, shaderResourceViews);
    ID3D11SamplerState* const samplers[1] = { samplerState_.get() };
    deviceContext_->PSSetSamplers(0, 1, samplers);

    // output merger
    ID3D11RenderTargetView* const renderTargetViews[1] = { renderTargetView_.get() };
    deviceContext_->OMSetRenderTargets(1, renderTargetViews, nullptr);
}

bool D3DRenderer::RenderFrame(const uint32_t* buffer)
{
    D3D11_MAPPED_SUBRESOURCE resource;
    auto hr = deviceContext_->Map(frameBuffer_.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    if (FAILED(hr))
        return false;

    auto src = reinterpret_cast<const uint8_t*>(buffer);
    auto srcPitch = width_ * 4;

    auto dst = reinterpret_cast<uint8_t*>(resource.pData);
    auto dstPitch = resource.RowPitch;

    for (auto y = 0u; y < height_; y++)
    {
        memcpy(dst, src, srcPitch);
        dst += dstPitch;
        src += srcPitch;
    }

    deviceContext_->Unmap(frameBuffer_.get(), 0);

    deviceContext_->DrawIndexed(6, 0, 0);

    hr = swapChain_->Present(1, 0);
    if (FAILED(hr))
        return false;

    return true;
}

bool D3DRenderer::CreateDevice()
{
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_9_1,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_12_1
    };

    D3D_FEATURE_LEVEL featureLevel;

#if DEBUG
    auto deviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
    auto deviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;
#endif
    auto hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        deviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        device_.put(),
        &featureLevel,
        deviceContext_.put());

    if (FAILED(hr))
        return false;

    return true;
}

bool D3DRenderer::CreateSwapChain(HWND window)
{
    winrt::com_ptr<IDXGIAdapter> adapter;
    auto hr = device_.as<IDXGIDevice>()->GetAdapter(adapter.put());
    if (FAILED(hr))
        return false;

    winrt::com_ptr<IDXGIFactory> factory;
    adapter->GetParent(IID_PPV_ARGS(factory.put()));

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = width_;
    swapChainDesc.BufferDesc.Height = height_;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.OutputWindow = window;

    hr = factory->CreateSwapChain(
        device_.get(),
        &swapChainDesc,
        swapChain_.put());
    if (FAILED(hr))
        return false;

    return true;
}

bool D3DRenderer::CreateRenderTarget()
{
    winrt::com_ptr<ID3D11Texture2D> backBuffer;
    auto hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.put()));
    if (FAILED(hr))
        return false;

    hr = device_->CreateRenderTargetView(backBuffer.get(), nullptr, renderTargetView_.put());
    if (FAILED(hr))
        return false;

    D3D11_TEXTURE2D_DESC backBufferDesc;
    backBuffer->GetDesc(&backBufferDesc);

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.Width = static_cast<FLOAT>(backBufferDesc.Width);
    viewport.Height = static_cast<FLOAT>(backBufferDesc.Height);
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;

    deviceContext_->RSSetViewports(1, &viewport);

    return true;
}

bool D3DRenderer::CreateVertexBuffer()
{
    const Vertex vertices[4] = {
        { -1.0f, 1.0f },
        { 1.0f, 1.0f },
        { -1.0f, -1.0 },
        { 1.0f, -1.0f },
    };

    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = sizeof(vertices);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));
    resourceData.pSysMem = vertices;

    auto hr = device_->CreateBuffer(&bufferDesc, &resourceData, vertexBuffer_.put());
    if (FAILED(hr))
        return false;

    return true;
}

bool D3DRenderer::CreateIndexBuffer()
{
    const uint16_t indices[6] = {
        0, 1, 2,
        1, 3, 2
    };

    D3D11_BUFFER_DESC bufferDesc;
    ZeroMemory(&bufferDesc, sizeof(bufferDesc));
    bufferDesc.ByteWidth = sizeof(uint16_t) * _countof(indices);
    bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA resourceData;
    ZeroMemory(&resourceData, sizeof(resourceData));
    resourceData.pSysMem = indices;

    auto hr = device_->CreateBuffer(&bufferDesc, &resourceData, indexBuffer_.put());
    if (FAILED(hr))
        return false;

    return true;
}

bool D3DRenderer::CreateFrameBuffer()
{
    D3D11_TEXTURE2D_DESC frameBufferDesc;
    ZeroMemory(&frameBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));
    frameBufferDesc.Width = width_;
    frameBufferDesc.Height = height_;
    frameBufferDesc.MipLevels = 1;
    frameBufferDesc.ArraySize = 1;
    frameBufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    frameBufferDesc.SampleDesc.Count = 1;
    frameBufferDesc.SampleDesc.Quality = 0;
    frameBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    frameBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    frameBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    frameBufferDesc.MiscFlags = 0;

    auto hr = device_->CreateTexture2D(&frameBufferDesc, nullptr, frameBuffer_.put());
    if (FAILED(hr))
        return false;

    return true;
}

bool D3DRenderer::CreateShaders()
{
    auto hr = device_->CreateVertexShader(
        VertexShaderBytecode,
        _countof(VertexShaderBytecode),
        nullptr,
        vertexShader_.put());

    if (FAILED(hr))
        return false;

    winrt::com_ptr<ID3D11PixelShader> pixelShader;
    hr = device_->CreatePixelShader(
        PixelShaderBytecode,
        _countof(PixelShaderBytecode),
        nullptr,
        pixelShader_.put());

    if (FAILED(hr))
        return false;

    return true;
}

bool D3DRenderer::CreateInputLayout()
{
    D3D11_INPUT_ELEMENT_DESC vertexLayoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    auto hr = device_->CreateInputLayout(
        vertexLayoutDesc,
        _countof(vertexLayoutDesc),
        VertexShaderBytecode,
        _countof(VertexShaderBytecode),
        inputLayout_.put());

    if (FAILED(hr))
        return false;

    return true;
}

bool D3DRenderer::CreateShaderParameters()
{
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
    ZeroMemory(&shaderResourceViewDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
    shaderResourceViewDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;

    auto hr = device_->CreateShaderResourceView(
        frameBuffer_.get(),
        &shaderResourceViewDesc,
        frameBufferShaderResourceView_.put());
    if (FAILED(hr))
        return false;

    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 1;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = device_->CreateSamplerState(&samplerDesc, samplerState_.put());
    if (FAILED(hr))
        return false;

    return true;
}

bool D3DRenderer::CreateRasterizerState()
{
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(ID3D11RasterizerState));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_BACK;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthBias = 0;
    rasterizerDesc.DepthBiasClamp = 0.0f;
    rasterizerDesc.SlopeScaledDepthBias = 0.0f;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = FALSE;
    rasterizerDesc.MultisampleEnable = FALSE;
    rasterizerDesc.AntialiasedLineEnable = FALSE;

    auto hr = device_->CreateRasterizerState(&rasterizerDesc, rasterizerState_.put());
    if (FAILED(hr))
        return false;

    return true;
}