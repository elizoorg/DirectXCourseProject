#pragma once
#include "Exports.h"
#include "GameComponent.h"
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include "ModelLoader.h"

class ENGINE_API ModelComponent:
	public GameComponent
{
    ID3D11InputLayout* layout;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11RasterizerState* rastState;

    ID3D11VertexShader* vertexShaderCSM;
    ID3D11GeometryShader* geomertyShaderCSM;

    ID3D11Buffer* g_pConstantBuffer11 = NULL;


    ID3D11Buffer* g_pConstantBuffer12 = NULL;


    D3D11_BUFFER_DESC cbDesc;
    D3D11_BUFFER_DESC cbcsmDesc;
    D3D11_SUBRESOURCE_DATA InitData;

    ID3D11SamplerState* TexSamplerState = nullptr;




    struct VS_CONSTANT_BUFFER
    {
        Matrix world;
        Matrix cameraView;
        Matrix cameraProj;
        Matrix InvWorldView;
    } buffer;





    // TODO: It doesnt work like that , points must declared somwhere else
    ID3DBlob* pixelB = nullptr;
    ID3DBlob* errorPixelCode = nullptr;

    ID3DBlob* vertexBC = nullptr;
    ID3DBlob* errorVertexCode = nullptr;


    ID3DBlob* geomertyBC = nullptr;
    ID3DBlob* errorGeomertyCode = nullptr;


    ID3DBlob* vertexBCSCM = nullptr;
    ID3DBlob* errorVectexCSMCode = nullptr;


    ID3D11Buffer* ib;
    ID3DBlob* pixelBC;



    ID3D11Buffer* vb;
    HRESULT res;

public:
    ModelComponent(Engine::Application* app) : GameComponent(app) {
    };
    ~ModelComponent();


    ModelLoader loader;

    void LoadModel(std::string path);


    void DestroyResources();
    void Reload();
    bool Initialize();
    virtual void Update(Matrix cameraProjection, Matrix cameraView, Matrix world, Matrix InverseView);
    void Update();
    void Draw();
    void PrepareFrame();

};

