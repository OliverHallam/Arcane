#pragma once

#define WIN32_LEAN_AND_MEAN

#include <cstdint>
#include <d3d11_4.h>

class D3DRenderer
{
public:
    D3DRenderer(uint32_t width, uint32_t height);

    void Initialize(HWND window);

    void PrepareRenderState();

    void RenderFrame(const uint32_t* buffer);

private:
    void CreateDevice();
    void CreateSwapChain(HWND window);
    void CreateRenderTarget();
    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateFrameBuffer();
    void CreateShaders();
    void CreateInputLayout();
    void CreateShaderParameters();
    void CreateRasterizerState();

    uint32_t width_;
    uint32_t height_;

    winrt::com_ptr<ID3D11Device> device_;
    winrt::com_ptr<ID3D11DeviceContext> deviceContext_;
    winrt::com_ptr<IDXGISwapChain> swapChain_;
    winrt::com_ptr<ID3D11RenderTargetView> renderTargetView_;

    winrt::com_ptr<ID3D11RasterizerState> rasterizerState_;

    winrt::com_ptr<ID3D11PixelShader> pixelShader_;
    winrt::com_ptr<ID3D11ShaderResourceView> frameBufferShaderResourceView_;
    winrt::com_ptr<ID3D11SamplerState> samplerState_;

    winrt::com_ptr<ID3D11VertexShader> vertexShader_;
    winrt::com_ptr<ID3D11InputLayout> inputLayout_;

    winrt::com_ptr<ID3D11Buffer> vertexBuffer_;
    winrt::com_ptr<ID3D11Buffer> indexBuffer_;
    winrt::com_ptr<ID3D11Texture2D> frameBuffer_;


    struct Vertex
    {
        float x;
        float y;
    };

};