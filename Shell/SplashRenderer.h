#pragma once

#include "LogoGenerator.h"

#include <d3d11_4.h>

class SplashRenderer
{
public:
    void Create(ID3D11Device* device);

    void PrepareRenderState(ID3D11DeviceContext* context) const;

    bool PrepareRender(ID3D11DeviceContext* context, float time);
    void Render(ID3D11DeviceContext* context);

private:
    void CreateVertexBuffer(ID3D11Device* device);
    void CreateIndexBuffer(ID3D11Device* device);

    void CreateShaders(ID3D11Device* device);
    void CreateInputLayout(ID3D11Device* device);


    winrt::com_ptr<ID3D11Buffer> vertexBuffer_;
    winrt::com_ptr<ID3D11Buffer> indexBuffer_;
    winrt::com_ptr<ID3D11PixelShader> pixelShader_;
    winrt::com_ptr<ID3D11VertexShader> vertexShader_;
    winrt::com_ptr<ID3D11InputLayout> inputLayout_;

    LogoGenerator generator_;
    int32_t indexCount_{};
};