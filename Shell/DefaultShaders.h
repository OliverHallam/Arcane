#pragma once

#include <d3d11_4.h>

class DefaultShaders
{
public:
    void Create(ID3D11Device* device);

    void PrepareRenderState(ID3D11DeviceContext* context) const;

private:
    void CreateShaders(ID3D11Device* device);
    void CreateInputLayout(ID3D11Device* device);
    
    winrt::com_ptr<ID3D11PixelShader> pixelShader_;

    winrt::com_ptr<ID3D11VertexShader> vertexShader_;
    winrt::com_ptr<ID3D11InputLayout> inputLayout_;
};