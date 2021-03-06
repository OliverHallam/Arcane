#pragma once

#include "DefaultShaders.h"
#include "ScanlineShaders.h"
#include "SplashRenderer.h"

#include <cstdint>
#include <d3d11_4.h>

class D3DRenderer
{
public:
    D3DRenderer();
    ~D3DRenderer();

    void Initialize(HWND window, uint32_t width, uint32_t height);

    void PrepareRenderState();

    bool WaitForFrame() const;
    uint64_t GetLastSyncTime(bool reset) const;

    bool RenderSplash(float time);
    void RenderSplash();

    void ClearFrameBuffer();

    void RenderFrame(const uint32_t* buffer, uint32_t refreshCycles);
    void RepeatLastFrame();
    void RenderClear();

    uint32_t RefreshRate(bool fullscreen) const;
    RECT GetFullscreenRect() const;

    void OnSize();

    void SetSplash(bool splash);
    void SetOverscan(bool overscan);
    void SetIntegerScaling(bool integerScaling);
    void SetScanlines(bool scanlines);

private:
    void CreateDevice();
    void CreateSwapChain();
    void CreateRenderTarget();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateFrameBuffer();
    void CreateRasterizerState();
    void CreateFramebufferSampler();

    void SetBuffers();

    void UpdateViewport();
    void UpdateViewport(int width, int height);

    uint32_t width_;
    uint32_t height_;

    bool overscan_;
    bool integerScaling_;
    bool scanlines_;
    bool splash_;

    winrt::com_ptr<ID3D11Device> device_;
    winrt::com_ptr<ID3D11DeviceContext> deviceContext_;

    winrt::com_ptr<IDXGISwapChain2> swapChain_;
    winrt::handle frameLatencyWaitableObject_;

    winrt::com_ptr<ID3D11RenderTargetView> renderTargetView_;

    winrt::com_ptr<ID3D11RasterizerState> rasterizerState_;

    winrt::com_ptr<ID3D11ShaderResourceView> frameBufferShaderResourceView_;

    winrt::com_ptr<ID3D11Buffer> vertexBuffer_;
    winrt::com_ptr<ID3D11Buffer> indexBuffer_;
    winrt::com_ptr<ID3D11Texture2D> frameBuffer_;

    DefaultShaders defaultShaders_;
    ScanlineShaders scanlineShaders_;

    SplashRenderer splashRenderer_;

    HWND window_;

};