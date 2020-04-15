#pragma once

#define WIN32_LEAN_AND_MEAN

#include <cstdint>
#include <d3d11_4.h>
#include <wrl/client.h>

class D3DRenderer
{
public:
    D3DRenderer(uint32_t width, uint32_t height);

    bool Initialize(HWND window);

    void PrepareRenderState();

    bool RenderFrame(const uint32_t* buffer);

private:
    bool CreateDevice();
    bool CreateSwapChain(HWND window);
    bool CreateRenderTarget();
    bool CreateVertexBuffer();
    bool CreateIndexBuffer();
    bool CreateFrameBuffer();
    bool CreateShaders();
    bool CreateInputLayout();
    bool CreateShaderParameters();
    bool CreateRasterizerState();

    uint32_t width_;
    uint32_t height_;

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView_;

    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;

    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> frameBufferShaderResourceView_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;

    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> frameBuffer_;


    struct Vertex
    {
        float x;
        float y;
    };

};