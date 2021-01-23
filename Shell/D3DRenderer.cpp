#include "pch.h"

#include "D3DRenderer.h"
#include "Error.h"

#include <dwmapi.h>

struct Vertex
{
    float x;
    float y;

    float tx;
    float ty;
};

D3DRenderer::D3DRenderer() :
    width_{},
    height_{},
    window_{ NULL },
    overscan_{},
    splash_{},
    scanlines_{},
    integerScaling_{ true }
{
}

D3DRenderer::~D3DRenderer()
{
    if (swapChain_)
        swapChain_->SetFullscreenState(FALSE, nullptr);
}

void D3DRenderer::Initialize(HWND window, uint32_t width, uint32_t height)
{
    window_ = window;
    width_ = width;
    height_ = height;

    CreateDevice();
    CreateSwapChain();
    CreateRenderTarget();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateFrameBuffer();
    CreateRasterizerState();

    CreateFramebufferSampler();

    defaultShaders_.Create(device_.get());
    scanlineShaders_.Create(device_.get());
    splashRenderer_.Create(device_.get());
}

void D3DRenderer::PrepareRenderState()
{
    deviceContext_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    SetBuffers();

    // rasterizer
    deviceContext_->RSSetState(rasterizerState_.get());

    // pixel shader
    ID3D11ShaderResourceView* const shaderResourceViews[1] = { frameBufferShaderResourceView_.get() };
    deviceContext_->PSSetShaderResources(0, 1, shaderResourceViews);

    defaultShaders_.PrepareRenderState(deviceContext_.get());

    UpdateViewport();
}

bool D3DRenderer::WaitForFrame() const
{
    auto result = WaitForSingleObjectEx(frameLatencyWaitableObject_.get(), 100, TRUE);
    return result == WAIT_OBJECT_0;
}

uint64_t D3DRenderer::GetLastSyncTime(bool reset) const
{
    DXGI_FRAME_STATISTICS stats;
    ZeroMemory(&stats, sizeof(DXGI_FRAME_STATISTICS));
    auto hr = swapChain_->GetFrameStatistics(&stats);

    if (!reset || hr != DXGI_ERROR_FRAME_STATISTICS_DISJOINT)
        winrt::check_hresult(hr);

    return stats.SyncQPCTime.QuadPart;
}

bool D3DRenderer::RenderSplash(float time)
{
    bool result = splashRenderer_.PrepareRender(deviceContext_.get(), time);

    RenderSplash();

    return result;
}

void D3DRenderer::RenderSplash()
{
    FLOAT grey[4]{ 0.75, 0.75, 0.75, 1.0 };
    deviceContext_->ClearRenderTargetView(renderTargetView_.get(), grey);

    auto renderTargetView = renderTargetView_.get();
    deviceContext_->OMSetRenderTargets(1, &renderTargetView, nullptr);

    splashRenderer_.Render(deviceContext_.get());

    auto hr = swapChain_->Present(1, 0);
    winrt::check_hresult(hr);
}

void D3DRenderer::ClearFrameBuffer()
{
    D3D11_MAPPED_SUBRESOURCE resource;
    auto hr = deviceContext_->Map(frameBuffer_.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    winrt::check_hresult(hr);

    ZeroMemory(resource.pData, resource.RowPitch * height_);

    deviceContext_->Unmap(frameBuffer_.get(), 0);
}

void D3DRenderer::RenderFrame(const uint32_t* buffer, uint32_t displayFrames)
{
    FLOAT grey[4]{ 0.125, 0.125, 0.125, 1.0 };
    deviceContext_->ClearRenderTargetView(renderTargetView_.get(), grey);

    auto renderTargetView = renderTargetView_.get();
    deviceContext_->OMSetRenderTargets(1, &renderTargetView, nullptr);

    D3D11_MAPPED_SUBRESOURCE resource;
    auto hr = deviceContext_->Map(frameBuffer_.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
    winrt::check_hresult(hr);

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

    deviceContext_->DrawIndexed(6, 0, overscan_ ? 4 : 0);

    hr = swapChain_->Present(displayFrames, 0);
    winrt::check_hresult(hr);
}

void D3DRenderer::RepeatLastFrame()
{
    if (splash_)
    {
        RenderSplash();
        return;
    }

    FLOAT grey[4] { 0.125, 0.125, 0.125, 1.0 };
    deviceContext_->ClearRenderTargetView(renderTargetView_.get(), grey);

    auto renderTargetView = renderTargetView_.get();
    deviceContext_->OMSetRenderTargets(1, &renderTargetView, nullptr);

    deviceContext_->DrawIndexed(6, 0, overscan_ ? 4 : 0);

    auto hr = swapChain_->Present(1, 0);
    winrt::check_hresult(hr);
}

void D3DRenderer::RenderClear()
{
    FLOAT grey[4]{ 0.125, 0.125, 0.125, 1.0 };
    deviceContext_->ClearRenderTargetView(renderTargetView_.get(), grey);

    auto hr = swapChain_->Present(1, 0);
    winrt::check_hresult(hr);
}

uint32_t D3DRenderer::RefreshRate(bool fullscreen) const
{
    if (fullscreen)
    {
        winrt::com_ptr<IDXGIOutput> output;
        auto hr = swapChain_->GetContainingOutput(output.put());
        winrt::check_hresult(hr);

        DXGI_OUTPUT_DESC desc;
        output->GetDesc(&desc);
        winrt::check_hresult(hr);

        MONITORINFOEX monitorInfo;
        ZeroMemory(&monitorInfo, sizeof(MONITORINFOEX));
        monitorInfo.cbSize = sizeof(MONITORINFOEX);
        if (!GetMonitorInfo(desc.Monitor, &monitorInfo))
            throw Error(L"Error in RefreshRate()");

        DEVMODE devMode;
        ZeroMemory(&devMode, sizeof(DEVMODE));
        devMode.dmSize = sizeof(DEVMODE);
        if (!EnumDisplaySettings(monitorInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode))
            throw Error(L"Error in RefreshRate()");

        return devMode.dmDisplayFrequency;
    }
    else
    {
        DWM_TIMING_INFO timingInfo;
        ZeroMemory(&timingInfo, sizeof(timingInfo));
        timingInfo.cbSize = sizeof(DWM_TIMING_INFO);

        auto hr = DwmGetCompositionTimingInfo(NULL, &timingInfo);
        winrt::check_hresult(hr);

        auto refreshRational = timingInfo.rateRefresh;
        return (refreshRational.uiNumerator + refreshRational.uiDenominator / 2) / refreshRational.uiDenominator;
    }
}

RECT D3DRenderer::GetFullscreenRect() const
{
    winrt::com_ptr<IDXGIOutput> output;
    auto hr = swapChain_->GetContainingOutput(output.put());
    winrt::check_hresult(hr);

    DXGI_OUTPUT_DESC desc;
    output->GetDesc(&desc);
    return desc.DesktopCoordinates;
}

void D3DRenderer::OnSize()
{
    if (swapChain_)
    {
        renderTargetView_.detach()->Release();

        swapChain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);

        CreateRenderTarget();

        UpdateViewport();
    }
}

void D3DRenderer::SetSplash(bool splash)
{
    splash_ = splash;

    UpdateViewport();

    if (splash)
    {
        splashRenderer_.PrepareRenderState(deviceContext_.get());
    }
    else
    {
        SetBuffers();
        if (scanlines_)
            scanlineShaders_.PrepareRenderState(deviceContext_.get());
        else
            defaultShaders_.PrepareRenderState(deviceContext_.get());
    }
}

void D3DRenderer::SetOverscan(bool overscan)
{
    overscan_ = overscan;

    if (!splash_)
        UpdateViewport();
}

void D3DRenderer::SetIntegerScaling(bool integerScaling)
{
    integerScaling_ = integerScaling;

    if (!splash_)
        UpdateViewport();
}

void D3DRenderer::SetScanlines(bool scanlines)
{
    scanlines_ = scanlines;

    if (splash_)
        return;

    if (scanlines)
        scanlineShaders_.PrepareRenderState(deviceContext_.get());
    else
        defaultShaders_.PrepareRenderState(deviceContext_.get());
}

void D3DRenderer::CreateDevice()
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

#ifdef _DEBUG
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

    winrt::check_hresult(hr);
}

void D3DRenderer::CreateSwapChain()
{
    winrt::com_ptr<IDXGIAdapter> adapter;
    auto hr = device_.as<IDXGIDevice>()->GetAdapter(adapter.put());
    winrt::check_hresult(hr);

    winrt::com_ptr<IDXGIFactory2> factory;
    adapter->GetParent(IID_PPV_ARGS(factory.put()));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
    swapChainDesc.Width = 0;
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;

    winrt::com_ptr<IDXGISwapChain1> swapChain;
    hr = factory->CreateSwapChainForHwnd(
        device_.get(),
        window_,
        &swapChainDesc,
        NULL,
        NULL,
        swapChain.put());
    winrt::check_hresult(hr);

    factory->MakeWindowAssociation(window_, DXGI_MWA_NO_ALT_ENTER);

    swapChain_ = swapChain.as<IDXGISwapChain2>();

    frameLatencyWaitableObject_.attach(swapChain_->GetFrameLatencyWaitableObject());

    winrt::check_hresult(hr);
}

void D3DRenderer::CreateRenderTarget()
{
    winrt::com_ptr<ID3D11Texture2D> backBuffer;
    auto hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.put()));
    winrt::check_hresult(hr);

    hr = device_->CreateRenderTargetView(backBuffer.get(), nullptr, renderTargetView_.put());
    winrt::check_hresult(hr);
}

void D3DRenderer::CreateVertexBuffer()
{
    auto h = 1.0f - 16.0f / width_;
    auto v = 1.0f - 16.0f / height_;

    const Vertex vertices[8] = {
        { -1.0f, 1.0f, -1.0f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f },
        { -1.0f, -1.0, -1.0f, -1.0f  },
        { 1.0f, -1.0f, 1.0f, -1.0f },

        // these are for when overscan is enabled.
        { -1.0f, 1.0f, -h, v },
        { 1.0f, 1.0f, h, v },
        { -1.0f, -1.0, -h, -v },
        { 1.0f, -1.0f, h, -v },
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
    winrt::check_hresult(hr);
}

void D3DRenderer::CreateIndexBuffer()
{
    const uint16_t indices[12] = {
        0, 1, 2,
        1, 3, 2,
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
    winrt::check_hresult(hr);
}

void D3DRenderer::CreateFrameBuffer()
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
    winrt::check_hresult(hr);
}

void D3DRenderer::CreateFramebufferSampler()
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
    winrt::check_hresult(hr);
}

void D3DRenderer::SetBuffers()
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    ID3D11Buffer* const vertexBuffers[1] = { vertexBuffer_.get() };
    deviceContext_->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
    deviceContext_->IASetIndexBuffer(indexBuffer_.get(), DXGI_FORMAT_R16_UINT, 0);
}

void D3DRenderer::CreateRasterizerState()
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
    winrt::check_hresult(hr);
}

void D3DRenderer::UpdateViewport()
{
    if (splash_)
        UpdateViewport(256, 96);
    else if (overscan_)
        UpdateViewport(width_ - 16, height_ - 16);
    else
        UpdateViewport(width_, height_);
}

void D3DRenderer::UpdateViewport(int width, int height)
{
    winrt::com_ptr<ID3D11Texture2D> backBuffer;
    auto hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.put()));
    winrt::check_hresult(hr);

    D3D11_TEXTURE2D_DESC backBufferDesc;
    backBuffer->GetDesc(&backBufferDesc);

    FLOAT viewportHeight, viewportWidth;
    if (integerScaling_ && !splash_)
    {
        // round down to nearest pixel multiple
        viewportHeight = static_cast<FLOAT>((backBufferDesc.Height / height) * height);
        viewportWidth = viewportHeight * width / height;

        if (viewportWidth > backBufferDesc.Width)
        {
            viewportWidth = static_cast<FLOAT>((backBufferDesc.Width / width) * width);
            viewportHeight = viewportWidth * height / width;
        }
    }
    else
    {
        viewportHeight = static_cast<FLOAT>(backBufferDesc.Height);
        viewportWidth = viewportHeight * width / height;

        if (viewportWidth > backBufferDesc.Width)
        {
            viewportWidth = static_cast<FLOAT>(backBufferDesc.Width);
            viewportHeight = viewportWidth * height / width;
        }
    }

    auto x = (backBufferDesc.Width - viewportWidth) / 2;
    auto y = (backBufferDesc.Height - viewportHeight) / 2;

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.Width = viewportWidth;
    viewport.Height = viewportHeight;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;

    deviceContext_->RSSetViewports(1, &viewport);
}
