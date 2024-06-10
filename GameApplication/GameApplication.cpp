#include "GameApplication.h"
#include <src/PlaneComponent.h>
#include <src/ModelComponent.h>
#include "external/SimpleMath.h"

	GameApplication::GameApplication()
	{
		instance = this;
		Device = new InputDevice(this);
		
	}
	GameApplication* GameApplication::instance = nullptr;
	GameApplication::~GameApplication()
	{

		for (auto comp : Components) {
			delete comp;
		}
	}


	int GameApplication::Run() {

		_display = new WinApi_Display(this);
		_display->CreateDisplay();

		Initialize();

		while (!isClosed) {
			UpdateInternal();
			Update();
			PrepareFrame();
			Draw();
			EndFrame();
		};


		return 0;

	}

	void GameApplication::CreateBackBuffer()
	{
	}

	void GameApplication::DestroyResources()
	{
	}

	void GameApplication::Draw()
	{

		context->ClearState();

		context->OMSetRenderTargets(1, &rtv, depthStencilView.Get());
		float color[] = { 0.6f, 0.6f, 0.6f, 1.0f };
		context->ClearRenderTargetView(rtv, color);
		context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		

		context->PSSetShaderResources(1, 1, &resource_view_depth_directional_light_);

		context->UpdateSubresource(LightDataBuffer.Get(), 0, nullptr, &LightData,0,0);

		context->UpdateSubresource(LightTransformBuffer.Get(), 0, nullptr, &LightTransform,0,0);

		context->VSSetConstantBuffers(1, 1, LightTransformBuffer.GetAddressOf());


		context->PSSetConstantBuffers(3, 1, LightDataBuffer.GetAddressOf());

		context->PSSetSamplers(1, 1, &sample_state_clamp_);



		for (size_t t = 0; t < 8; t++) {
			Vector3 scale, pos;
			Quaternion rot;
			Matrix world = transform[t].GetWorldMatrix();
			world.Decompose(scale, rot, pos);

			Components[t]->Update(camera->Proj(), camera->View(), transform[t].GetWorldMatrix(),
				(Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rot)).Invert().Transpose());
		}
	

		Vector3 Pscale, Ppos;
		Quaternion Prot;
		Matrix world = playerTransform.GetWorldMatrix();
		world.Decompose(Pscale, Prot, Ppos);
		player->Update(camera->Proj(), camera->View(), playerTransform.GetWorldMatrix(),
			(Matrix::CreateScale(Pscale) * Matrix::CreateFromQuaternion(Prot)).Invert().Transpose());


		for (size_t t = 0; t < Components.size(); t++)
		{
			Components[t]->Draw();
		}

		context->OMSetBlendState(blendState_.Get(), NULL, 0xFFFFFFFF);
		player->Draw();

		context->ClearState();
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		D3D11_VIEWPORT viewport = {};
		viewport.Width = 600;
		viewport.Height = 600;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1.0f;
		context->RSSetViewports(1, &viewport);

		manager->SetShader(ShaderData("./Shaders/test.hlsl", Vertex | Pixel));

		context->RSSetState(rastState_.Get());
		context->OMSetRenderTargets(1, &rtv, depthStencilView.Get());
		context->PSSetSamplers(0, 1, &TexSamplerState);
		context->PSSetShaderResources(0, 1, &resource_view_depth_directional_light_);
		context->Draw(3, 0);
	}

	void GameApplication::EndFrame()
	{
		context->OMSetRenderTargets(0, nullptr, nullptr);
		swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);
	}

	void GameApplication::Exit()
	{
	}

	void GameApplication::Initialize()
	{
		Device = new InputDevice(this);
		camera = new Camera(this);
		camera->SetPosition(0,1, -10);
		camera->Bind();
	
		swapDesc.BufferCount = 2;
		swapDesc.BufferDesc.Width = _display->getWidth();
		swapDesc.BufferDesc.Height = _display->getHeight();
		swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.OutputWindow = _display->getHWND();
		swapDesc.Windowed = true;
		swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapDesc.SampleDesc.Count = 1;
		swapDesc.SampleDesc.Quality = 0;


		D3D_FEATURE_LEVEL featureLevel[] = { D3D_FEATURE_LEVEL_11_1 };
		auto res = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			D3D11_CREATE_DEVICE_DEBUG,
			featureLevel,
			1,
			D3D11_SDK_VERSION,
			&swapDesc,
			&swapChain,
			&device,
			nullptr,
			&context);
		if (FAILED(res))
		{
			std::cout << "So,unexpected shit happens\n";
			// Well, that was unexpected
		}
		res = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backTex);	// __uuidof(ID3D11Texture2D)
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens3\n";
		}
		res = device->CreateRenderTargetView(backTex, nullptr, &rtv);
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens2\n";
		}

		
		ModelComponent* model = new ModelComponent(this);
		ModelComponent* model2 = new ModelComponent(this);
		ModelComponent* model3 = new ModelComponent(this);
		ModelComponent* model4 = new ModelComponent(this);
		ModelComponent* model5 = new ModelComponent(this);
		ModelComponent* model6 = new ModelComponent(this);
		ModelComponent* model7 = new ModelComponent(this);
		ModelComponent* model8 = new ModelComponent(this);
		ModelComponent* model9 = new ModelComponent(this);

		Components.push_back(model);
		Components.push_back(model2);
		Components.push_back(model3);
		Components.push_back(model4);
		Components.push_back(model5);
		Components.push_back(model6);
		Components.push_back(model7);
		Components.push_back(model8);
		Components.push_back(model9);


		for (auto comp : Components) {
			comp->Initialize();
		}

		

		system = new DebugRenderSysImpl(this);
		system->SetCamera(camera);

		rastDesc.CullMode = D3D11_CULL_FRONT;
		rastDesc.FillMode = D3D11_FILL_SOLID;


		std::srand(std::time(nullptr));

		ResetGame();


		res = device->CreateRasterizerState(&rastDesc, &rastState);
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens4\n";
		}

		D3D11_TEXTURE2D_DESC depthStencilDesc;
		depthStencilDesc.Width = _display->getWidth();
		depthStencilDesc.Height = _display->getHeight();
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;
		depthStencilDesc.SampleDesc = { 1,0 };

		D3D11_DEPTH_STENCIL_DESC depthstencildesc;


		depthstencildesc.DepthEnable = true;
		depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthstencildesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		depthstencildesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP; //KEEP
		depthstencildesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR; //INCR
		depthstencildesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; //KEEP
		depthstencildesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		depthstencildesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP; //KEEP
		depthstencildesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR; //DECR
		depthstencildesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP; //KEEP
		depthstencildesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	

		res = device->CreateTexture2D(&depthStencilDesc, NULL, depthStencilBuffer.GetAddressOf());
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens5\n";
		}

		res = device->CreateDepthStencilView(depthStencilBuffer.Get(), NULL, depthStencilView.GetAddressOf());
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens6\n";
		}


		res = device->CreateDepthStencilState(&depthstencildesc, depthStencilState.GetAddressOf());
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens7\n";
		}


		static_cast<ModelComponent*>(Components[0])->LoadModel("assets/Plane/untitled.obj");
		static_cast<ModelComponent*>(Components[1])->LoadModel("assets/Hawk/10025_Hawk_v1_iterations-2.obj");
		static_cast<ModelComponent*>(Components[2])->LoadModel("assets/Hawk/10025_Hawk_v1_iterations-2.obj");
		static_cast<ModelComponent*>(Components[3])->LoadModel("assets/Hawk/10025_Hawk_v1_iterations-2.obj");
		static_cast<ModelComponent*>(Components[4])->LoadModel("assets/Hawk/10025_Hawk_v1_iterations-2.obj");
		static_cast<ModelComponent*>(Components[5])->LoadModel("assets/Hawk/10025_Hawk_v1_iterations-2.obj");
		static_cast<ModelComponent*>(Components[6])->LoadModel("assets/Hawk/10025_Hawk_v1_iterations-2.obj");
		static_cast<ModelComponent*>(Components[7])->LoadModel("assets/Hawk/10025_Hawk_v1_iterations-2.obj");
	


		D3D11_TEXTURE2D_DESC depthDescription = {};
		depthDescription.Width = 2048;
		depthDescription.Height = 2048;
		depthDescription.ArraySize = 5;
		depthDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		depthDescription.Format = DXGI_FORMAT_R32_TYPELESS;
		depthDescription.MipLevels = 1;
		depthDescription.SampleDesc.Count = 1;
		depthDescription.SampleDesc.Quality = 0;
		depthDescription.Usage = D3D11_USAGE_DEFAULT;
		depthDescription.CPUAccessFlags = 0;
		depthDescription.MiscFlags = 0;

		res = device->CreateTexture2D(&depthDescription, nullptr, shadowTextureArray.GetAddressOf());

		if (FAILED(res))
		{
			OutputDebugString(TEXT("Fatal error: Failed to create CSM depth texture array!\n"));
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC dViewDesc = {};
		dViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dViewDesc.Texture2DArray = {};
		dViewDesc.Texture2DArray.MipSlice = 0;
		dViewDesc.Texture2DArray.FirstArraySlice = 0;
		dViewDesc.Texture2DArray.ArraySize = 5;

		res = device->CreateDepthStencilView(shadowTextureArray.Get(), &dViewDesc, depthShadowDsv.GetAddressOf());

		if (FAILED(res))
		{
			OutputDebugString(TEXT("Fatal error: Failed to create CSM depth stencil view!\n"));
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

		res = device->CreateSamplerState(&depthSamplerStateDesc, &depthSamplerState_);


		if (FAILED(res))
		{
			OutputDebugString(TEXT("Cannot make depth sampler state!\n"));
		}

		D3D11_DEPTH_STENCIL_DESC defaultDepthDesc = {};

		defaultDepthDesc.DepthEnable = true;
		defaultDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		defaultDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;

		res = device->CreateDepthStencilState(&defaultDepthDesc, defaultDepthState_.GetAddressOf());

		manager = new ShaderManager(this);

		CD3D11_RASTERIZER_DESC rastDesc = {};

		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FillMode = D3D11_FILL_SOLID;
		rastDesc.FrontCounterClockwise = true;
		rastDesc.DepthClipEnable = true;


		res = device->CreateRasterizerState(&rastDesc, rastState_.GetAddressOf());

		D3D11_DEPTH_STENCIL_DESC quadDepthDesc = {};

		defaultDepthDesc.DepthEnable = true;
		defaultDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		defaultDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;

		res = device->CreateDepthStencilState(&quadDepthDesc, quadDepthState_.GetAddressOf());

		D3D11_BLEND_DESC blendDesc = {};

		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; //D3D11_BLEND_SRC_COLOR;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; //D3D11_BLEND_BLEND_FACTOR;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
		blendDesc.AlphaToCoverageEnable = false;

		res = device->CreateBlendState(&blendDesc, blendState_.GetAddressOf());



		manager->InitShader(ShaderData("./Shaders/DepthShader.hlsl", Vertex | Pixel));
		manager->InitShader(ShaderData("./Shaders/Shader.hlsl", Vertex | Pixel));
		manager->InitShader(ShaderData("./Shaders/GBuffer.hlsl", Vertex | Pixel));
		manager->InitShader(ShaderData("./Shaders/LightVolume.hlsl", Vertex | Pixel));
		manager->InitShader(ShaderData("./Shaders/test.hlsl", Vertex | Pixel));


		player = new SphereComponent(this);
		player->Initialize();
		player->LoadTexture(L"assets/defaulttexture.png");


		auto rot = Matrix::CreateLookAt(playerTransform.GetWorldPosition() + offset, playerTransform.GetWorldPosition(), Vector3(0, 1, 0)).ToEuler();
		rot.x *= -1;
		rot.z = 0;
		camera->getTransform()->SetEulerRotate(rot);


		lightPos =
			Vector3::Transform(Vector3(-60, 40, 0), Quaternion::CreateFromAxisAngle(Vector3::Left, 0)); 

		std::cout << lightPos.x << " " << lightPos.y << " " << lightPos.z << std::endl;

		auto directional_light_direction = Vector4(
			-lightPos.x, -lightPos.y, -lightPos.z, 0);
		directional_light_direction.Normalize();
		std::cout << directional_light_direction.x << " " << directional_light_direction.y << " " << directional_light_direction.z << std::endl;
		directional_light_view = Matrix::CreateLookAt(
			lightPos, lightPos + Vector3::Right + Vector3::Down, Vector3::Up);

		LightData.color = Vector4(1, 1, 1, 0);
		LightData.diffK_specA_specK = Vector4(1.0, 100.0f, 1.0f, 0);
		LightData.direction = directional_light_direction;

		directional_light_projection_ = Matrix::CreatePerspectiveFieldOfView(3.14f / 2,camera->ASPECT_RATIO, 0.01f, 100);


		LightTransform.directional_light_view_projection = directional_light_view * directional_light_projection_;

		D3D11_BUFFER_DESC constBufPerSceneDesc;
		constBufPerSceneDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBufPerSceneDesc.Usage = D3D11_USAGE_DEFAULT;
		constBufPerSceneDesc.CPUAccessFlags = 0;
		constBufPerSceneDesc.MiscFlags = 0;
		constBufPerSceneDesc.StructureByteStride = 0;
		constBufPerSceneDesc.ByteWidth = sizeof(DirectionalLightBufferElement);
		res = device->CreateBuffer(&constBufPerSceneDesc, nullptr, LightDataBuffer.GetAddressOf());
		if (FAILED(res))
		{
			OutputDebugString(TEXT("Fatal error: Failed to create LightDataBuffer!\n"));
		}


		D3D11_BUFFER_DESC constBufPerLightDecs;
		constBufPerLightDecs.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBufPerLightDecs.Usage = D3D11_USAGE_DEFAULT;
		constBufPerLightDecs.CPUAccessFlags = 0;
		constBufPerLightDecs.MiscFlags = 0;
		constBufPerLightDecs.StructureByteStride = 0;
		constBufPerLightDecs.ByteWidth = sizeof(LightTransform);
		res = device->CreateBuffer(&constBufPerLightDecs, nullptr, LightTransformBuffer.GetAddressOf());
		if (FAILED(res))
		{
			OutputDebugString(TEXT("Fatal error: Failed to create LightTransformBuffer!\n"));
		}

		D3D11_TEXTURE2D_DESC depth_stencil_descriptor;
		depth_stencil_descriptor.Width = _display->getWidth() * 3;
		depth_stencil_descriptor.Height = _display->getHeight() * 3;
		depth_stencil_descriptor.MipLevels = 1;
		depth_stencil_descriptor.ArraySize = 1;
		depth_stencil_descriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_descriptor.SampleDesc.Count = 1;
		depth_stencil_descriptor.SampleDesc.Quality = 0;
		depth_stencil_descriptor.Usage = D3D11_USAGE_DEFAULT;
		depth_stencil_descriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depth_stencil_descriptor.CPUAccessFlags = 0;
		depth_stencil_descriptor.MiscFlags = 0;


		ID3D11Texture2D* depth_stencil_buffer;
		device->CreateTexture2D(&depth_stencil_descriptor, nullptr, &depth_stencil_buffer);
		device->CreateDepthStencilView(depth_stencil_buffer, nullptr, &depth_stencil_view_);



		D3D11_TEXTURE2D_DESC depth_texture_descriptor{};
		depth_texture_descriptor.Width = _display->getWidth() * 3;
		depth_texture_descriptor.Height = _display->getHeight() * 3;
		depth_texture_descriptor.MipLevels = 1;
		depth_texture_descriptor.ArraySize = 1;
		depth_texture_descriptor.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		depth_texture_descriptor.SampleDesc.Count = 1;
		depth_texture_descriptor.Usage = D3D11_USAGE_DEFAULT;
		depth_texture_descriptor.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		ID3D11Texture2D* background_texture_depth;
		device->CreateTexture2D(&depth_texture_descriptor, nullptr, &background_texture_depth);
		device->CreateRenderTargetView(background_texture_depth, nullptr, &render_target_view_depth_directional_light_);
		device->CreateShaderResourceView(background_texture_depth, nullptr, &resource_view_depth_directional_light_);

		D3D11_SAMPLER_DESC sampler_descriptor;
		sampler_descriptor.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_descriptor.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_descriptor.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_descriptor.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampler_descriptor.MipLODBias = 0.0f;
		sampler_descriptor.MaxAnisotropy = 1;
		sampler_descriptor.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampler_descriptor.BorderColor[0] = 0;
		sampler_descriptor.BorderColor[1] = 0;
		sampler_descriptor.BorderColor[2] = 0;
		sampler_descriptor.BorderColor[3] = 0;
		sampler_descriptor.MinLOD = 0;
		sampler_descriptor.MaxLOD = D3D11_FLOAT32_MAX;
		device->CreateSamplerState(&sampler_descriptor, &sample_state_clamp_);

		sampler_descriptor.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampler_descriptor.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_descriptor.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_descriptor.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampler_descriptor.MipLODBias = 0.0f;
		sampler_descriptor.MaxAnisotropy = 1;
		sampler_descriptor.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampler_descriptor.BorderColor[0] = 0;
		sampler_descriptor.BorderColor[1] = 0;
		sampler_descriptor.BorderColor[2] = 0;
		sampler_descriptor.BorderColor[3] = 0;
		sampler_descriptor.MinLOD = 0;
		sampler_descriptor.MaxLOD = D3D11_FLOAT32_MAX;

		device->CreateSamplerState(&sampler_descriptor, &sample_state_wrap_);



		D3D11_SAMPLER_DESC sampDesc{};
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		//sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampDesc.BorderColor[0] = 1.0f;
		sampDesc.BorderColor[1] = 0.0f;
		sampDesc.BorderColor[2] = 0.0f;
		sampDesc.BorderColor[3] = 1.0f;
		sampDesc.MaxLOD = static_cast<float>(INT_MAX);
		sampDesc.MipLODBias = 0.f;
		sampDesc.MaxAnisotropy = 1;



		//sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;

		res = device->CreateSamplerState(&sampDesc, &TexSamplerState);
		if (FAILED(res)) {
			std::cout << "Something is going wrong with texture sampler";
		}

	}

	void GameApplication::MessageHandler()
	{
	}

	void GameApplication::PrepareFrame()
	{

		context->ClearState();
		constexpr float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		context->ClearRenderTargetView(render_target_view_depth_directional_light_, color);
		context->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
		context->OMSetRenderTargets(1, &render_target_view_depth_directional_light_, depth_stencil_view_);
		
		for (auto comp : Components)
			comp->PrepareFrame();
		
		player->PrepareFrame();


	}

	void GameApplication::PrepareRecources()
	{
	}

	void GameApplication::RestoreTargets()
	{
		context->OMSetRenderTargets(1, &rtv, depthStencilView.Get());
	}

	bool GameApplication::Update()
	{

		while (!_display->PollMessages())
		{

		}
		auto tr = camera->getTransform();
		if (Device->IsKeyDown(Keys::Escape))
		{
			isClosed = true;
			return true;
		}


		Vector3 direction = Vector3(0, 0, 0);


		Vector3 right = tr->GetRightVector();
		right.y = 0;
		Vector3 forward = tr->GetForwardVector();
		forward.y = 0;
		auto rot = playerTransform.GetQuaternionRotate();

		if (Device->IsKeyDown(Keys::D))
		{
			playerTransform.AdjustPosition(right);
			rot *= Quaternion::CreateFromAxisAngle(forward, 4.0f * deltaTime);
			direction += tr->GetRightVector();
		}
		if (Device->IsKeyDown(Keys::A))
		{
			playerTransform.AdjustPosition(right * -1);
			rot *= Quaternion::CreateFromAxisAngle(-forward, 4.0f * deltaTime);
			direction += tr->GetRightVector() * -1;
		}
		if (Device->IsKeyDown(Keys::S))
		{
			playerTransform.AdjustPosition(forward * -1);
			rot *= Quaternion::CreateFromAxisAngle(right, 4.0f * deltaTime);
			direction += tr->GetForwardVector() * -1;
		}
		if (Device->IsKeyDown(Keys::W))
		{
			playerTransform.AdjustPosition(forward);
			rot *= Quaternion::CreateFromAxisAngle(-right, 4.0f * deltaTime);
			direction += tr->GetForwardVector();
		}
		if (Device->IsKeyDown(Keys::J))
		{
			isMouseUsed = !isMouseUsed;
		}
		if (isMouseUsed) {
			SetCursorPos(_display->getWidth() / 2, _display->getHeight() / 2);
		}


		const auto offset_ = Vector3::Transform(offset, Matrix::CreateFromQuaternion(camera->getTransform()->GetQuaternionRotate()));

		camera->getTransform()->SetPosition(playerTransform.GetWorldPosition() + offset_);

		PlayerCollision.Center = playerTransform.GetWorldPosition();


		playerTransform.SetQuaternionRotate(rot);
		camera->Update();


		for (size_t t = 0; t < foodSpheres.size(); t++) {
			if (PlayerCollision.Intersects(foodSpheres[t])) {
				transform[t+1].SetParent(&playerTransform);
				Vector3 playerPos = playerTransform.GetWorldPosition();
				Quaternion quat = playerTransform.GetQuaternionRotate();
				Quaternion conj = quat;
				conj.Conjugate();
				Vector3 localPos = transform[t+1].GetWorldPosition();
				localPos = localPos - playerPos;
				localPos = XMVector3Rotate(localPos, conj);
				foodSpheres[t].Center = Vector3(1000, 1000, 1000);
				collected.push_back(std::make_pair(t+1, localPos));


			}
		}
		for (auto&& pair : collected) 
		{
			int a = pair.first;
			transform[a].SetPosition(pair.second);
			Vector3 direction = pair.second.Cross(Vector3::UnitY);
			transform[a].SetQuaternionRotate(Quaternion::CreateFromAxisAngle(direction, XM_PIDIV2));
		}



		for (size_t t = 7; t >= 1; t--)
			transform[t].Update();
		
		for (size_t t = 0; t < 8; t++) {
			Vector3 scale, pos;
			Quaternion rot;
			Matrix world = transform[t].GetWorldMatrix();
			world.Decompose(scale, rot, pos);

			Components[t]->Update(directional_light_projection_ ,directional_light_view, transform[t].GetWorldMatrix(),
				(Matrix::CreateScale(scale)*Matrix::CreateFromQuaternion(rot)).Invert().Transpose());
		}

		
		Vector3 Pscale, Ppos;
		Quaternion Prot;
		Matrix world = playerTransform.GetWorldMatrix();
		world.Decompose(Pscale, Prot, Ppos);
		player->Update(directional_light_projection_, directional_light_view, playerTransform.GetWorldMatrix(),
			(Matrix::CreateScale(Pscale)* Matrix::CreateFromQuaternion(Prot)).Invert().Transpose());



		Vector3 campos = camera->getTransform()->GetWorldPosition();
		LightTransform.camera_position = Vector4(campos.x,campos.y,campos.z,1.0f) ;

		



		return true;

	}

	void GameApplication::UpdateInternal()
	{
		auto	curTime = std::chrono::steady_clock::now();
		deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;

		totalTime += deltaTime;
		frameCount++;

		if (totalTime > 1.0f) {
			float fps = frameCount / totalTime;

			totalTime -= 1.0f;

			WCHAR text[256];
			swprintf_s(text, TEXT("FPS: %f"), fps);
			SetWindowText(_display->getHWND(), text);

			frameCount = 0;
		}
	}
	void GameApplication::ResetGame()
	{

		//Plane transform
		transform[0].SetPosition(Vector3(50, 0, -50));
		transform[0].SetEulerRotate(Vector3(0, 0, 0));
		transform[0].SetScale(Vector3(20, 1, 20));

		//Models transforms
		for (size_t t = 1; t < 8; t++) {
			transform[t].SetEulerRotate(Vector3(0, 90, 0));
			transform[t].SetScale(Vector3(0.5f, 0.5f, 0.5f));
		}


		transform[1].SetPosition(Vector3(50, 0, 0));
		transform[2].SetPosition(Vector3(25, 0, 25));
		transform[3].SetPosition(Vector3 ( -30, 0, -30));
		transform[4].SetPosition(Vector3(25, 0, -40));
		transform[5].SetPosition(Vector3(-50, 0, 10));
		transform[6].SetPosition(Vector3(30, 0, -15));
		transform[7].SetPosition(Vector3(-45, 0, 35));

		//Models Bounding Spheres
		BoundingSphere center;
		center.Center = transform[1].GetWorldPosition() +Vector3(0,10,0);
		center.Radius = gameSize/2;
		foodSpheres.push_back(center);
		center.Center = transform[2].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);
		center.Center = transform[3].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);
		center.Center = transform[4].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);
		center.Center = transform[5].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);
		center.Center = transform[6].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);
		center.Center = transform[7].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);
 
		//Player Transform
		playerTransform.SetPosition(Vector3(0, gameSize, 0));
		playerTransform.SetScale(Vector3(gameSize));
		playerTransform.SetEulerRotate(Vector3(0, 0, 0));

		//Player Bounding
		PlayerCollision.Center = playerTransform.GetWorldPosition();
		PlayerCollision.Radius = playerTransform.GetScale().x;

		


	}

