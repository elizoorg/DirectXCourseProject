#pragma once
#include "Exports.h"
#include "GameComponent.h"
#include "../external/SimpleMath.h"
#include "MathTypes.h"

class TriangleComponent :
    public GameComponent
{
private:
    ID3D11InputLayout* layout;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11RasterizerState* rastState;

    ID3D11Buffer* g_pConstantBuffer11 = NULL;
    D3D11_BUFFER_DESC cbDesc;
    D3D11_SUBRESOURCE_DATA InitData;


    struct VS_CONSTANT_BUFFER
    {
        Matrix gWorldViewProj;
        Vector4 offset;
        Vector4 scale;
    } buffer;


    // TODO: It doesnt work like that , points must declared somwhere else
    DirectX::XMFLOAT4 points[8] = {
        Vector4(-3.0f, 3.0f, -3.0f,1.0f),
        Vector4(3.0f, 3.0f, -3.0f,1.0f),
        Vector4(-3.0f, -3.0f, -3.0f,1.0f),
        Vector4(3.0f, -3.0f, -3.0f,1.0f),
        Vector4(-3.0f, 3.0f, 3.0f,1.0f),
        Vector4(3.0f, 3.0f, 3.0f,1.0f),
        Vector4(-3.0f, -3.0f, 3.0f,1.0f),
        Vector4(3.0f, -3.0f, 3.0f,1.0f)
    };

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
    void Update(DirectX::SimpleMath::Matrix mat,Vector4 offset, Vector4 scale);
    void Update();
    void Draw();
};

