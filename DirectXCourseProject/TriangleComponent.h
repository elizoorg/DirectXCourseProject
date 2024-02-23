#pragma once
#include "Exports.h"
#include "GameComponent.h"
#include "SimpleMath.h"


class TriangleComponent :
    public GameComponent
{
private:
    ID3D11InputLayout* layout;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11RasterizerState* rastState;
    // TODO: It doesnt work like that , points must declared somwhere else
    DirectX::XMFLOAT4 points[8] = {DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
        DirectX::XMFLOAT4(-0.5f, -0.5f, 0.5f, 1.0f),	DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),
        DirectX::XMFLOAT4(0.5f, -0.5f, 0.5f, 1.0f),	DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
        DirectX::XMFLOAT4(-0.5f, 0.5f, 0.5f, 1.0f),	DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), };

    ID3DBlob* pixelB = nullptr;
    ID3DBlob* errorPixelCode =nullptr;

    ID3DBlob* vertexBC = nullptr;
    ID3DBlob* errorVertexCode = nullptr;


    ID3D11Buffer* ib;
    ID3DBlob* pixelBC;

  

    ID3D11Buffer* vb;
    HRESULT res;

public:
    TriangleComponent(Engine::Application* app) : GameComponent(app) {
    };
    ~TriangleComponent();
    void DestroyResources();
    void Reload();
    bool Initialize();
    void Update(DirectX::SimpleMath::Matrix mat);
    void Update();
    void Draw();
};

