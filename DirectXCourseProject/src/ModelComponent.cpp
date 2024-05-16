#include "ModelComponent.h"
#include "Application.h"


ModelComponent::~ModelComponent()
{
	DestroyResources();
}



void ModelComponent::LoadModel(std::string path)
{
	loader.Load(_app->getDisplay()->getHWND(), _app->getDevice().Get(), _app->getContext(), path);
}

void ModelComponent::DestroyResources()
{
	g_pConstantBuffer11->Release();
	ib->Release();
	vb->Release();

}

void ModelComponent::Reload()
{
}

bool ModelComponent::Initialize()
{
	res = D3DCompileFromFile(L"./Shaders/TexturedShaderCSM.hlsl",
		nullptr /*macros*/,
		nullptr /*include*/,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexBC,
		&errorVertexCode);

	if (FAILED(res))
	{
		// If the shader failed to compile it should have written something to the error message.
		if (errorVertexCode)
		{
			auto compileErrors = static_cast<char*>(errorVertexCode->GetBufferPointer());

			std::cout <<"1"<< compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(_app->getDisplay()->getHWND(), L"TexturedShaderCSM.hlsl", L"Missing Shader File", MB_OK);
		}

		return false;
	}

	D3D_SHADER_MACRO Shader_Macros[] = { "1", "TCOLOR", "float4(0.0f, 1.0f, 0.0f, 1.0f)", nullptr, nullptr };

	res = D3DCompileFromFile(L"./Shaders/TexturedShaderCSM.hlsl", Shader_Macros /*macros*/, nullptr /*include*/,
		"PSMain", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pixelBC,
		&errorPixelCode);

	if (FAILED(res))
	{
		// If the shader failed to compile it should have written something to the error message.
		if (errorPixelCode)
		{
			auto compileErrors = static_cast<char*>(errorVertexCode->GetBufferPointer());

			std::cout <<"2"<< compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(_app->getDisplay()->getHWND(), L"TexturedShaderCSM.hlsl", L"Missing Shader File", MB_OK);
		}

		return false;
	}

	res = D3DCompileFromFile(L"./Shaders/csm.hlsl", Shader_Macros /*macros*/, nullptr /*include*/,
		"GSMain", "gs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &geomertyBC,
		&errorGeomertyCode);

	if (FAILED(res))
	{
		// If the shader failed to compile it should have written something to the error message.
		if (errorGeomertyCode)
		{
			auto compileErrors = static_cast<char*>(errorGeomertyCode->GetBufferPointer());

			std::cout <<"3"<< compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(_app->getDisplay()->getHWND(), L"Csm.hlsl", L"Missing Shader File", MB_OK);
		}

		return false;
	}

	res = D3DCompileFromFile(L"./Shaders/csm.hlsl", Shader_Macros /*macros*/, nullptr /*include*/,
		"VSMain", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &vertexBCSCM,
		&errorVectexCSMCode);

	if (FAILED(res))
	{
		// If the shader failed to compile it should have written something to the error message.
		if (errorVectexCSMCode)
		{
			auto compileErrors = static_cast<char*>(errorVectexCSMCode->GetBufferPointer());

			std::cout <<"4"<< compileErrors << std::endl;
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(_app->getDisplay()->getHWND(), L"Csm.hlsl", L"Missing Shader File", MB_OK);
		}

		return false;
	}



	D3D11_INPUT_ELEMENT_DESC inputElements[] = {
		D3D11_INPUT_ELEMENT_DESC{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		D3D11_INPUT_ELEMENT_DESC{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		},
		D3D11_INPUT_ELEMENT_DESC{
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};


	_app->getDevice()->CreateInputLayout(
		inputElements,
		3,
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		&layout);



	_app->getDevice()->CreateVertexShader(
		vertexBC->GetBufferPointer(),
		vertexBC->GetBufferSize(),
		nullptr, &vertexShader);

	_app->getDevice()->CreatePixelShader(
		pixelBC->GetBufferPointer(),
		pixelBC->GetBufferSize(),
		nullptr, &pixelShader);

	_app->getDevice()->CreateVertexShader(
		vertexBCSCM->GetBufferPointer(),
		vertexBCSCM->GetBufferSize(),
		nullptr, &vertexShaderCSM);

	_app->getDevice()->CreateGeometryShader(
		geomertyBC->GetBufferPointer(),
		geomertyBC->GetBufferSize(),
		nullptr, &geomertyShaderCSM);



	CD3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.AntialiasedLineEnable = false;
	rastDesc.CullMode = D3D11_CULL_BACK;
	rastDesc.DepthBias = 0;
	rastDesc.DepthBiasClamp = 0.0f;
	rastDesc.DepthClipEnable = true;
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.FrontCounterClockwise = false;
	rastDesc.MultisampleEnable = false;
	rastDesc.ScissorEnable = false;
	rastDesc.SlopeScaledDepthBias = 0.0f;


	//ID3D11RasterizerState* rastState;
	res = _app->getDevice()->CreateRasterizerState(&rastDesc, &rastState);
	

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	res = _app->getDevice()->CreateSamplerState(&sampDesc, &TexSamplerState);
	if (FAILED(res)) {
		std::cout << "Something is going wrong with texture sampler";
	}
		
	D3D11_SAMPLER_DESC depthSamplerStateDesc = {};
	depthSamplerStateDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	depthSamplerStateDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;
	depthSamplerStateDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	depthSamplerStateDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	depthSamplerStateDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	depthSamplerStateDesc.BorderColor[0] = 1.0f;
	depthSamplerStateDesc.BorderColor[1] = 1.0f;
	depthSamplerStateDesc.BorderColor[2] = 1.0f;
	depthSamplerStateDesc.BorderColor[3] = 1.0f;

	res = _app->getDevice()->CreateSamplerState(&depthSamplerStateDesc, &depthSamplerState_);


}

void ModelComponent::Update(Matrix cameraProjection, Matrix cameraView, Matrix world, Matrix InverseView)
{
	if (g_pConstantBuffer11) {
		g_pConstantBuffer11->Release();
	}
	buffer.cameraProj = cameraProjection;
	buffer.cameraView = cameraView;
	buffer.world = world;
	buffer.InvWorldView = InverseView;

	cbDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Fill in the subresource data.

	InitData.pSysMem = &buffer;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;



	_app->getDevice()->CreateBuffer(&cbDesc, &InitData,
		&g_pConstantBuffer11);


	cbcsmDesc.ByteWidth = sizeof(Matrix) * 5 + sizeof(Vector4);
	cbcsmDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbcsmDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbcsmDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbcsmDesc.MiscFlags = 0;
	cbcsmDesc.StructureByteStride = 0;

	// Fill in the subresource data.

	_app->getDevice()->CreateBuffer(&cbcsmDesc, nullptr,
		&g_pConstantBuffer12);

}


void ModelComponent::Update()
{

}

void ModelComponent::Draw()
{
	_app->getContext()->RSSetState(rastState);


	//_app->getContext()->OMSetDepthStencilState(_app->getStencilState().Get(), 0);


	_app->getContext()->VSSetShader(vertexShader, nullptr, 0);
	_app->getContext()->PSSetShader(pixelShader, nullptr, 0);

	_app->getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_app->getContext()->IASetInputLayout(layout);


	_app->getContext()->PSSetSamplers(0, 1, &TexSamplerState);
	_app->getContext()->PSSetSamplers(1, 1, &depthSamplerState_);

	auto csm = _app->getCSM();


	_app->getContext()->PSSetShaderResources(1, 1, &csm);

	_app->getContext()->UpdateSubresource(g_pConstantBuffer11, 0, nullptr, &buffer, 0, 0);
	_app->getContext()->VSSetConstantBuffers(0, 1, &g_pConstantBuffer11);
	_app->getContext()->PSSetConstantBuffers(0, 1, _app->getLightBuffer().GetAddressOf());



	loader.Draw(_app->getContext());
}

void ModelComponent::PrepareFrame()
{
	_app->getContext()->RSSetState(rastState);

	D3D11_VIEWPORT viewport = {};
	viewport.Width = 1024.0f;
	viewport.Height = 1024.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0f;

	_app->getContext()->RSSetViewports(1, &viewport);

	_app->getContext()->IASetInputLayout(layout);
	_app->getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_app->getContext()->VSSetShader(vertexShaderCSM, nullptr, 0);
	_app->getContext()->VSSetConstantBuffers(0, 1, &g_pConstantBuffer11);

	_app->getContext()->PSSetShader(nullptr, nullptr, 0);

	_app->getContext()->GSSetShader(geomertyShaderCSM, nullptr, 0);
	_app->getContext()->GSSetConstantBuffers(0, 1, &g_pConstantBuffer12);


	loader.Draw(_app->getContext());
}