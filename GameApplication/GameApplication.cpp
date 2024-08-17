// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "GameApplication.h"
#include <src/PlaneComponent.h>
#include <src/ModelComponent.h>
#include "external/SimpleMath.h"

	GameApplication::GameApplication()
	{
		instance = this;
		Device = new InputDevice(this);
		gBuffer_ = new GBuffer(this);
		
	}
	GameApplication* GameApplication::instance = nullptr;
	GameApplication::~GameApplication()
	{

		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		for (auto comp : Components) {
			delete comp;
		}
	}


	int GameApplication::Run() {

		_display = new WinApi_Display(this);
		_display->CreateDisplay();
		//_display.OnMouseMove += [](InputDevice::RawMouseEventArgs args) {InputDevice::Instance().OnMouseMove(args); };
		//_display.OnKeyDown += [](InputDevice::KeyboardInputEventArgs args) {InputDevice::Instance().OnKeyDown(args); };





		Initialize();

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplWin32_Init(_display->getHWND());
		ImGui_ImplDX11_Init(device.Get(), context);
		ImGui::StyleColorsDark();
		io.WantCaptureMouse = true;


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
		context->OMSetDepthStencilState(defaultDepthState_.Get(), 0);


		const auto rtvs = new ID3D11RenderTargetView * [3];
		rtvs[0] = gBuffer_->albedoRtv_.Get();
		rtvs[1] = gBuffer_->positionRtv_.Get();
		rtvs[2] = gBuffer_->normalRtv_.Get();
		context->OMSetRenderTargets(3, rtvs, depthStencilView.Get());

		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };

		context->ClearRenderTargetView(gBuffer_->albedoRtv_.Get(), color);
		context->ClearRenderTargetView(gBuffer_->positionRtv_.Get(), color);
		context->ClearRenderTargetView(gBuffer_->normalRtv_.Get(), color);
		context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		for (size_t t = 23; t < 30; t++)
		{
			Components[t]->Draw();
		}

		player->Draw();

		context->ClearState();

		context->RSSetState(rastState_.Get());
		context->OMSetDepthStencilState(quadDepthState_.Get(), 0);


		



		context->ClearRenderTargetView(postProcessRtv_.Get(), color);
		context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
		context->OMSetRenderTargets(1, postProcessRtv_.GetAddressOf(), depthStencilView.Get());

		//context->ClearRenderTargetView(rtv, color);
		




		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(_display->getWidth());
		viewport.Height = static_cast<float>(_display->getHeight());
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1.0f;

		context->RSSetViewports(1, &viewport);

		//context->OMSetRenderTargets(1, &rtv, nullptr);

		context->OMSetBlendState(nullptr, nullptr, 0xffffffff);

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		manager->SetShader(ShaderData("./Shaders/TextureShaderDerref.hlsl", Vertex | Pixel));

		context->PSSetShaderResources(0, 1, gBuffer_->albedoSrv_.GetAddressOf());
		context->PSSetShaderResources(1, 1, gBuffer_->positionSrv_.GetAddressOf());
		context->PSSetShaderResources(2, 1, gBuffer_->normalSrv_.GetAddressOf());
		context->PSSetShaderResources(3, 1, depthShadowSrv.GetAddressOf());
		context->PSSetShaderResources(4, 1, &defaultTexture.texture);

		context->PSSetConstantBuffers(1, 1, LightBuffer.GetAddressOf());
		context->PSSetConstantBuffers(2, 1, cascadeCBuffer_.GetAddressOf());
		context->PSSetSamplers(0, 1, depthSamplerState_.GetAddressOf());
		context->PSSetSamplers(1, 1, &TexSamplerState);



		context->Draw(4, 0);

		context->OMSetBlendState(blendState_.Get(), nullptr, 0xffffffff);
	
		lightData.AmbientSpecularRowType = Vector4(0.4f, 0.5f, 32, 1);

		lightData.LightPos = Vector4(3, 2, 3, 1);
		lightData.LightPos = Vector4::Transform(lightData.LightPos, camera->View());
		lightData.LightColor = Vector4(1, 0, 0, 1) * 2.0f;

		context->UpdateSubresource(LightBuffer.Get(), 0, nullptr, &lightData, 0, 0);


		lightTransform.SetScale(Vector3((10.0f)));
		lightTransform.SetPosition(Vector3(3, 2, 3));
		lightTransform.SetEulerRotate(Vector3(0, 0, 0));
		lightTransform.Update();

		Vector3 scale, pos;
		Quaternion rot;
		lightTransform.GetWorldMatrix().Decompose(scale, rot, pos);



		volume->Update(camera->Proj(), camera->View(), lightTransform.GetWorldMatrix(),
			(Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rot)).Invert().Transpose());

		volume->Draw();

		

		lightData.LightPos = Vector4(-3, 1, -3, 1);
		lightData.LightPos = Vector4::Transform(lightData.LightPos, camera->View());
		lightData.LightColor = Vector4(0, 1, 0, 1) * 2.0f;

		context->UpdateSubresource(LightBuffer.Get(), 0, nullptr, &lightData, 0, 0);


		lightTransform.SetScale(Vector3((10.0f)));
		lightTransform.SetPosition(Vector3(-3, 1, -3));


		lightTransform.GetWorldMatrix().Decompose(scale, rot, pos);
		lightTransform.Update();


		volume->Update(camera->Proj(), camera->View(), lightTransform.GetWorldMatrix(),
			(Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rot)).Invert().Transpose());

		volume->Draw();


		lightData.LightPos = Vector4(3, 1, -3, 1);
		lightData.LightPos = Vector4::Transform(lightData.LightPos, camera->View());
		lightData.LightColor = Vector4(0, 0,1, 1) * 2.0f;

		context->UpdateSubresource(LightBuffer.Get(), 0, nullptr, &lightData, 0, 0);


		lightTransform.SetScale(Vector3((10.0f)));
		lightTransform.SetPosition(Vector3(3, 1, -3));
		lightTransform.Update();

		lightTransform.GetWorldMatrix().Decompose(scale, rot, pos);



		volume->Update(camera->Proj(), camera->View(), lightTransform.GetWorldMatrix(),
			(Matrix::CreateScale(scale) * Matrix::CreateFromQuaternion(rot)).Invert().Transpose());

		volume->Draw();

		/*for (size_t t = 24; t < 30; t++)
		{
			system->DrawSphere(10, Color(255, 0, 0), transform[t].GetWorldMatrix() , 50);
		}

		system->DrawSphere(1, Color(255, 0, 0), playerTransform.GetWorldMatrix(), 50);
		system->DrawLine(Vector3(0, 0, 0), Vector3(0, 200, 0), Color(0, 255, 0, 255));
		system->DrawLine(Vector3(0, 0, 0), Vector3(200, 0, 0), Color(255, 0, 0, 255));
		system->DrawLine(Vector3(0, 0, 0), Vector3(0, 0, 200), Color(0, 0, 255, 255));
		
		system->Draw(deltaTime);*/

		system->Clear();
		
		bool qq = false;

		viewport = {};
		viewport.Width = 1920;
		viewport.Height = 1080;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1.0f;


		context->RSSetViewports(1, &viewport);


		manager->SetShader(ShaderData("./Shaders/post.hlsl", Vertex | Pixel));



		context->OMSetRenderTargets(1, &rtv, depthStencilView.Get());


		context->PSSetSamplers(0, 1, &TexSamplerState);

		context->ClearRenderTargetView(rtv, color);

		context->PSSetShaderResources(0, 1, postProcessSrv_.GetAddressOf());


		context->Draw(3, 0);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();

		ImGui::NewFrame();

	

		ImGui::Begin("Testestestestestestest");
		bool isHovered = ImGui::IsItemHovered();
		bool isFocused = ImGui::IsItemFocused();
		ImVec2 mousePositionAbsolute = ImGui::GetMousePos();
		ImVec2 screenPositionAbsolute = ImGui::GetItemRectMin();
		ImVec2 mousePositionRelative = ImVec2(mousePositionAbsolute.x - screenPositionAbsolute.x, mousePositionAbsolute.y - screenPositionAbsolute.y);
		ImGui::Text("Is mouse over screen? %s", isHovered ? "Yes" : "No");
		ImGui::Text("Is screen focused? %s", isFocused ? "Yes" : "No");
		ImGui::Text("Position: %f, %f", mousePositionRelative.x, mousePositionRelative.y);
		ImGui::Text("Mouse clicked: %s", ImGui::IsMouseDown(ImGuiMouseButton_Left) ? "Yes" : "No");


		ImGui::Text("%f %f %f %f", camera->getTransform()->GetWorldPosition().x, camera->getTransform()->GetWorldPosition().y,camera->getTransform()->GetWorldPosition().z, 1.0f);

		ImGui::Text("%f %f %f %f", transform[0].GetWorldPosition().x, transform[0].GetWorldPosition().y, transform[0].GetWorldPosition().z, planets[0].radius);
		ImGui::Text("%f %f %f %f", transform[1].GetWorldPosition().x, transform[1].GetWorldPosition().y, transform[1].GetWorldPosition().z, planets[1].radius);
		ImGui::Text("%f %f %f %f", transform[2].GetWorldPosition().x, transform[2].GetWorldPosition().y, transform[2].GetWorldPosition().z, planets[2].radius);
		ImGui::Text("%f %f %f %f", transform[3].GetWorldPosition().x, transform[3].GetWorldPosition().y, transform[3].GetWorldPosition().z, planets[3].radius);
		ImGui::Text("%f %f %f %f", transform[4].GetWorldPosition().x, transform[4].GetWorldPosition().y, transform[4].GetWorldPosition().z, planets[4].radius);
		ImGui::Text("%f %f %f %f", transform[5].GetWorldPosition().x, transform[5].GetWorldPosition().y, transform[5].GetWorldPosition().z, planets[5].radius);
		ImGui::Text("%f %f %f %f", transform[6].GetWorldPosition().x, transform[6].GetWorldPosition().y, transform[6].GetWorldPosition().z, planets[6].radius);



		ImGui::SliderFloat("Test", &angle, 0.0f, 6.28f);
		ImGui::SliderFloat("Platen0", &planets[0].angle, 0.0f, 360.28f);
		ImGui::SliderFloat("Platen1", &planets[1].angle, 0.0f, 360.28f);
		ImGui::SliderFloat("Platen2", &planets[2].angle, 0.0f, 360.28f);
		ImGui::SliderFloat("Platen3", &planets[3].angle, 0.0f, 360.28f);
		ImGui::SliderFloat("Platen4", &planets[4].angle, 0.0f, 360.28f);
		ImGui::SliderFloat("Platen5", &planets[5].angle, 0.0f, 360.28f);
		ImGui::SliderFloat("Platen6", &planets[6].angle, 0.0f, 360.28f);


		ImGui::SliderFloat("Scale sphere x", &playerTransform.localScale.x, 0.0f,30.0f );
		ImGui::SliderFloat("Scale sphere y", &playerTransform.localScale.y, 0.0f,30.0f );
		ImGui::SliderFloat("Scale sphere z", &playerTransform.localScale.z, 0.0f,30.0f );


		ImGui::SliderFloat("Cat x", &transform[29].localScale.x, 0.0f, 30.0f);
		ImGui::SliderFloat("Cat y", &transform[29].localScale.y, 0.0f, 30.0f);
		ImGui::SliderFloat("Cat z", &transform[29].localScale.z, 0.0f, 30.0f);





		ImGui::SliderFloat("Direction of light x", &tmp.x, 0.0f, 360.28f);
		ImGui::SliderFloat("Direction of light y", &tmp.y, 0.0f, 360.28f);
		ImGui::SliderFloat("Direction of light z ", &tmp.z, 0.0f, 360.28f);
		ImGui::SliderFloat("Direction of light w", &tmp.w, 0.0f, 360.28f);


		ImGui::End();
		ImGui::Render();

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

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


		savedRot= { 0.f, 0.f, 0.f, 1.f };



		Device = new InputDevice(this);
		camera = new Camera(this);
		camera->SetPosition(0,1, -10);
		camera->Bind();
		light = new DirectionalLight(this);
	
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

		SphereComponent* sphere = new SphereComponent(this);
		SphereComponent* sphere2 = new SphereComponent(this);
		SphereComponent* sphere3 = new SphereComponent(this);
		SphereComponent* sphere4 = new SphereComponent(this);
		SphereComponent* sphere5 = new SphereComponent(this);
		SphereComponent* sphere6 = new SphereComponent(this);
		SphereComponent* sphere7 = new SphereComponent(this);


		TriangleComponent* cube = new TriangleComponent(this);
		TriangleComponent* cube2 = new TriangleComponent(this);
		TriangleComponent* cube3 = new TriangleComponent(this);
		TriangleComponent* cube4 = new TriangleComponent(this);
		TriangleComponent* cube5 = new TriangleComponent(this);
		TriangleComponent* cube6 = new TriangleComponent(this);
		TriangleComponent* cube7 = new TriangleComponent(this);
		TriangleComponent* cube8 = new TriangleComponent(this);
		TriangleComponent* cube9 = new TriangleComponent(this);
		TriangleComponent* cube10 = new TriangleComponent(this);
		TriangleComponent* cube11 = new TriangleComponent(this);
		TriangleComponent* cube12 = new TriangleComponent(this);
		TriangleComponent* cube13 = new TriangleComponent(this);
		TriangleComponent* cube14 = new TriangleComponent(this);
		TriangleComponent* cube15 = new TriangleComponent(this);
		PlaneComponent* plane = new PlaneComponent(this);


		ModelComponent* model = new ModelComponent(this);
		ModelComponent* model2 = new ModelComponent(this);
		ModelComponent* model3 = new ModelComponent(this);
		ModelComponent* model4 = new ModelComponent(this);
		ModelComponent* model5 = new ModelComponent(this);
		ModelComponent* model6 = new ModelComponent(this);
		ModelComponent* model7 = new ModelComponent(this);
		ModelComponent* model8 = new ModelComponent(this);

		Components.push_back(sphere);
		Components.push_back(sphere2);
		Components.push_back(sphere3);
		Components.push_back(sphere4);
		Components.push_back(sphere5);
		Components.push_back(sphere6);
		Components.push_back(sphere7);

		Components.push_back( cube );
		Components.push_back( cube2 );
		Components.push_back( cube3 );
		Components.push_back( cube4 );
		Components.push_back( cube5 );
		Components.push_back( cube6 );
		Components.push_back( cube7 );
		Components.push_back( cube8 );
		Components.push_back( cube9 );
		Components.push_back( cube10);
		Components.push_back( cube11);
		Components.push_back( cube12);
		Components.push_back( cube13);
		Components.push_back( cube14);
		Components.push_back( cube15);

		Components.push_back(plane);

		Components.push_back(model);
		Components.push_back(model2);
		Components.push_back(model3);
		Components.push_back(model4);
		Components.push_back(model5);
		Components.push_back(model6);
		Components.push_back(model7);
		Components.push_back(model8);


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


		static_cast<ModelComponent*>(Components[23])->LoadModel("assets/Plane/untitled.obj");
		static_cast<ModelComponent*>(Components[24])->LoadModel("assets/cat2/12221_Cat_v1_l3.obj");
		static_cast<ModelComponent*>(Components[25])->LoadModel("assets/cat2/12221_Cat_v1_l3.obj");
		static_cast<ModelComponent*>(Components[26])->LoadModel("assets/cat2/12221_Cat_v1_l3.obj");
		static_cast<ModelComponent*>(Components[27])->LoadModel("assets/cat2/12221_Cat_v1_l3.obj");
		static_cast<ModelComponent*>(Components[28])->LoadModel("assets/cat2/12221_Cat_v1_l3.obj");
		static_cast<ModelComponent*>(Components[29])->LoadModel("assets/cat2/12221_Cat_v1_l3.obj");
		static_cast<ModelComponent*>(Components[30])->LoadModel("assets/cat2/12221_Cat_v1_l3.obj");
	


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

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray = {};
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = 5;

		res = device->CreateShaderResourceView(shadowTextureArray.Get(), &srvDesc, depthShadowSrv.GetAddressOf());

		if (FAILED(res))
		{
			OutputDebugString(TEXT("Fatal error: Failed to create CSM depth SRV!\n"));
		}

		D3D11_BUFFER_DESC constBufPerSceneDesc;
		constBufPerSceneDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBufPerSceneDesc.Usage = D3D11_USAGE_DEFAULT;
		constBufPerSceneDesc.CPUAccessFlags = 0;
		constBufPerSceneDesc.MiscFlags = 0;
		constBufPerSceneDesc.StructureByteStride = 0;
		constBufPerSceneDesc.ByteWidth = sizeof(LightData);
		res = device->CreateBuffer(&constBufPerSceneDesc, nullptr, LightBuffer.GetAddressOf());
		if (FAILED(res))
		{
			OutputDebugString(TEXT("Fatal error: Failed to create CSM depth SRV!\n"));
		}
		D3D11_BUFFER_DESC constBufCascadeDesc;
		constBufCascadeDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		constBufCascadeDesc.Usage = D3D11_USAGE_DEFAULT;
		constBufCascadeDesc.CPUAccessFlags = 0;
		constBufCascadeDesc.MiscFlags = 0;
		constBufCascadeDesc.StructureByteStride = 0;
		constBufCascadeDesc.ByteWidth = sizeof(Matrix) * 5 + sizeof(Vector4);

		device->CreateBuffer(&constBufCascadeDesc, nullptr, cascadeCBuffer_.GetAddressOf());

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



		gBuffer_->Initialize();
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
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE; //D3D11_BLEND_SRC_COLOR;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE; //D3D11_BLEND_BLEND_FACTOR;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;
		blendDesc.AlphaToCoverageEnable = false;

		res = device->CreateBlendState(&blendDesc, blendState_.GetAddressOf());



		manager->InitShader(ShaderData("./Shaders/csm.hlsl", Vertex | Geometry));
		manager->InitShader(ShaderData("./Shaders/TextureShaderDerref.hlsl", Vertex | Pixel));
		manager->InitShader(ShaderData("./Shaders/GBuffer.hlsl", Vertex | Pixel));
		manager->InitShader(ShaderData("./Shaders/LightVolume.hlsl", Vertex | Pixel));
		manager->InitShader(ShaderData("./Shaders/post.hlsl", Vertex | Pixel));

		volume = new LightComponent(this);
		volume->Initialize();

		player = new SphereComponent(this);
		player->Initialize();

		player->LoadTexture(L"assets/defaulttexture.png");



		
		auto rot = Matrix::CreateLookAt(playerTransform.GetWorldPosition() + offset, playerTransform.GetWorldPosition(), Vector3(0, 1, 0)).ToEuler();
		rot.x *= -1;
		rot.z = 0;
		camera->getTransform()->SetEulerRotate(rot);



		depthDescription.Width = _display->getWidth();
		depthDescription.Height = _display->getHeight();
		depthDescription.ArraySize = 1;
		depthDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		depthDescription.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		depthDescription.MipLevels = 1;
		depthDescription.SampleDesc.Count = 1;
		depthDescription.SampleDesc.Quality = 0;
		depthDescription.Usage = D3D11_USAGE_DEFAULT;
		depthDescription.CPUAccessFlags = 0;
		depthDescription.MiscFlags = 0;

		res = device->CreateTexture2D(&depthDescription, nullptr, postProcessBuffer_.GetAddressOf());


		device->CreateRenderTargetView(postProcessBuffer_.Get(), nullptr, postProcessRtv_.GetAddressOf());
		device->CreateShaderResourceView(postProcessBuffer_.Get(), nullptr, postProcessSrv_.GetAddressOf());



		D3D11_SAMPLER_DESC sampDesc{};
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
		//sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		sampDesc.BorderColor[0] = 1.0f;
		sampDesc.BorderColor[1] = 0.0f;
		sampDesc.BorderColor[2] = 0.0f;
		sampDesc.BorderColor[3] = 1.0f;
		sampDesc.MaxLOD = static_cast<float>(INT_MAX);
		sampDesc.MipLODBias = 0.f;
		sampDesc.MaxAnisotropy = 1;



		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;

		res = device->CreateSamplerState(&sampDesc, &TexSamplerState);
		if (FAILED(res)) {
			std::cout << "Something is going wrong with texture sampler";
		}


		const HRESULT result = CreateWICTextureFromFile(device.Get(),
			context,L"assets/shape.png", nullptr, &defaultTexture.texture);


	
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

		context->OMSetRenderTargets(0, nullptr, depthShadowDsv.Get());

		context->ClearDepthStencilView(depthShadowDsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		
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
			//tr->AdjustPosition(tr->GetRightVector());
			playerTransform.AdjustPosition(right);
			rot *= Quaternion::CreateFromAxisAngle(forward, 4.0f * deltaTime);
			direction += tr->GetRightVector();
		}
		if (Device->IsKeyDown(Keys::A))
		{
			//tr->AdjustPosition(tr->GetRightVector()*-1);
			playerTransform.AdjustPosition(right * -1);
			rot *= Quaternion::CreateFromAxisAngle(-forward, 4.0f * deltaTime);
			direction += tr->GetRightVector() * -1;
		}
		if (Device->IsKeyDown(Keys::S))
		{
			//tr->AdjustPosition(tr->GetForwardVector()*-1);
			playerTransform.AdjustPosition(forward * -1);
			rot *= Quaternion::CreateFromAxisAngle(right, 4.0f * deltaTime);
			direction += tr->GetForwardVector() * -1;
		}
		if (Device->IsKeyDown(Keys::W))
		{
			//tr->AdjustPosition(tr->GetForwardVector());
			playerTransform.AdjustPosition(forward);
			rot *= Quaternion::CreateFromAxisAngle(-right, 4.0f * deltaTime);
			direction += tr->GetForwardVector();
		}
		if (Device->IsKeyDown(Keys::Space))
		{
			//tr->AdjustPosition(tr->GetUpVector());
		}
		if (Device->IsKeyDown(Keys::J))
		{
			isMouseUsed = !isMouseUsed;
		}
		if (Device->IsKeyDown(Keys::LeftShift))
		{
			//tr->AdjustPosition(tr->GetUpVector()*-1);
		}
		if (Device->IsKeyDown(Keys::LeftButton)) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseButtonEvent(0, true);
		}
		else {
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseButtonEvent(0, false);
		}
		if (isMouseUsed) {
			SetCursorPos(_display->getWidth() / 2, _display->getHeight() / 2);
		}




		if (Device->IsKeyDown(Keys::LeftControl) || Device->IsKeyDown(Keys::RightAlt)) {
			std::cout << "It's working!\n";
		}

		auto test = tmp;
		test.Normalize();
		direction.Normalize();
		light->SetDirection(test);
		lightData.LightPos = Vector4::Transform(light->GetDirection(), camera->View());
		lightData.LightPos.Normalize();
		lightData.LightColor = light->GetColor();
		lightData.AmbientSpecularRowType = Vector4(0.4f, 0.5f, 32, 0);
		lightData.T = Matrix(
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, -0.5f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.5f, 0.5f, 0.0f, 1.0f);
		lightData.ViewMatrix = camera->View();

		auto tmp2 = getLight()->GetLightSpaceMatrices();


		for (int i = 0; i < 5; ++i)
		{
			cascadeData.ViewProj[i] = tmp2[i];
		}
		cascadeData.Distance = getLight()->GetShadowCascadeDistances();

		context->UpdateSubresource(cascadeCBuffer_.Get(), 0, nullptr, &cascadeData, 0, 0);
		context->UpdateSubresource(LightBuffer.Get(), 0, nullptr, &lightData, 0, 0);

		const auto offset_ = Vector3::Transform(offset, Matrix::CreateFromQuaternion(camera->getTransform()->GetQuaternionRotate()));

		camera->getTransform()->SetPosition(playerTransform.GetWorldPosition() + offset_);

		PlayerCollision.Center = playerTransform.GetWorldPosition();


		playerTransform.SetQuaternionRotate(rot);
		camera->Update();
		for (size_t t = 0; t < foodSpheres.size(); t++) {
			if (PlayerCollision.Intersects(foodSpheres[t])) {
				transform[t + 24].SetParent(&playerTransform);



				//Matrix worldPos = playerTransform.GetWorldMatrix().Transpose().Invert();
				Vector3 playerPos = playerTransform.GetWorldPosition();
				Quaternion quat = playerTransform.GetQuaternionRotate();
				Quaternion conj = quat;
				conj.Conjugate();
				//Vector3 catPos = transform[t].GetWorldPosition();
				//Vector3 localPos = DirectX::SimpleMath::Vector3::Transform(catPos, worldPos);


				Vector3 localPos = transform[t + 24].GetWorldPosition();
				localPos = localPos - playerPos;
				//Quaternion result = quat * Quaternion(localPos.x, localPos.y, localPos.z, 0) * conj;
				//localPos = Vector3(result.x, result.y, result.z);
				localPos = XMVector3Rotate(localPos, conj);

				Matrix trans = playerTransform.GetWorldMatrix().Invert();
				Matrix trans2 =  transform[t + 24].GetWorldMatrix() * trans;

				Vector3 Pscale, Ppos;
				Quaternion Prot;
				trans2.Decompose(Pscale, Prot, Ppos);

				transform[t + 24].SetPosition(Ppos);
				transform[t + 24].SetScale(Pscale);
				transform[t + 24].SetQuaternionRotate(Prot);
				transform[t + 24].Update();



				//localPos = 3. * localPos / 4.;
				foodSpheres[t].Center = Vector3(1000, 1000, 1000);
				collected.push_back(std::make_pair(t + 24, localPos));


			}


			

		}
		for (auto&& pair : collected) 
		{
			//int a = pair.first;
			//transform[a].SetPosition(Vector3::Transform(pair.second,playerTransform.GetWorldMatrix()));
			//transform[a].SetPosition(pair.second);

			//Vector3 direction = pair.second.Cross(Vector3::UnitY);
			//transform[a].SetQuaternionRotate(Quaternion::CreateFromAxisAngle(direction, XM_PIDIV2));
			/*
			Matrix lol = transform[a].GetWorldMatrix();
			DirectX::SimpleMath::Matrix thisToWorld = playerTransform.GetWorldMatrix();

			transform[a].SetPosition(DirectX::SimpleMath::Vector3::Transform(pair.second, thisToWorld)); 
			*/
			/*
			Quaternion l = transform[a].GetQuaternionRotate();
			if (direction.Length() > 0)
			{
				l *= Quaternion::CreateFromAxisAngle(-direction, 4.0f * deltaTime);
			}

			transform[a].SetQuaternionRotate(l);
			*/
		}



		for (size_t t = 29; t >= 24; t--)
			transform[t].Update();
		
		for (size_t t = 0; t < 30; t++) {
			Vector3 scale, pos;
			Quaternion rot;
			Matrix world = transform[t].GetWorldMatrix();
			world.Decompose(scale, rot, pos);

			Components[t]->Update(camera->Proj(), camera->View(), transform[t].GetWorldMatrix(),
				(Matrix::CreateScale(scale)*Matrix::CreateFromQuaternion(rot)).Invert().Transpose());
		}

		
		Vector3 Pscale, Ppos;
		Quaternion Prot;
		Matrix world = playerTransform.GetWorldMatrix();
		world.Decompose(Pscale, Prot, Ppos);
		player->Update(camera->Proj(), camera->View(), playerTransform.GetWorldMatrix(),
			(Matrix::CreateScale(Pscale)* Matrix::CreateFromQuaternion(Prot)).Invert().Transpose());

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
		for (size_t t = 0; t < 22; t++) {
			transform[t].SetPosition(Vector3(0, 0, 0));
		}
		



		transform[0].SetScale(Vector3(15.0f,15.0f,15.0f));
		transform[1].SetScale(Vector3(15.0f,15.0f,15.0f));
		transform[2].SetScale(Vector3(15.0f,15.0f,15.0f));
		transform[3].SetScale(Vector3(15.0f,15.0f,15.0f));
		transform[4].SetScale(Vector3(15.0f,15.0f,15.0f));	
		transform[5].SetScale(Vector3(15.0f,15.0f,15.0f));
		transform[6].SetScale(Vector3(30.0f,30.0f,30.0f));

		transform[22].SetPosition(Vector3(0, 0, 0));
		transform[22].SetScale(Vector3(2.0f, 2.0f, 2.0f));
		transform[22].SetEulerRotate(Vector3(0, 0, 0));


		for (size_t t = 7; t < 22; t++) {
			transform[t].SetPosition(Vector3(2.0f, 2.0f, 2.0f));
		}

		for (size_t t = 7; t < 22; t++) {
			planets[t].angleSpeed = rand_FloatRange(-0.5, 0.5);
			planets[t].angleSpeed2 = rand_FloatRange(-0.5, 0.5);
			planets[t].radius = rand_FloatRange(10, 20);
		}

	



		planets[0].angleSpeed = rand_FloatRange(-0.5, 0.5);
		planets[1].angleSpeed = rand_FloatRange(-0.5, 0.5);
		planets[2].angleSpeed = rand_FloatRange(-0.5, 0.5);
		planets[3].angleSpeed = rand_FloatRange(-0.5, 0.5);
		planets[4].angleSpeed = rand_FloatRange(-0.5, 0.5);
		planets[5].angleSpeed = rand_FloatRange(-0.5, 0.5);
		planets[6].angleSpeed = rand_FloatRange(-1, 1);
		planets[0].angleSpeed2 = rand_FloatRange(-0.5, 0.5);
		planets[1].angleSpeed2 = rand_FloatRange(-0.5, 0.5);
		planets[2].angleSpeed2 = rand_FloatRange(-0.5, 0.5);
		planets[3].angleSpeed2 = rand_FloatRange(-0.5, 0.5);
		planets[4].angleSpeed2 = rand_FloatRange(-0.5, 0.5);
		planets[5].angleSpeed2 = rand_FloatRange(-0.5, 0.5);
		planets[6].angleSpeed2 = rand_FloatRange(-1, 1);

		planets[0].radius = rand_FloatRange(30, 90);
		planets[1].radius = rand_FloatRange(30, 90);
		planets[2].radius = rand_FloatRange(30, 90);
		planets[3].radius = rand_FloatRange(30, 90);
		planets[4].radius = rand_FloatRange(30, 90);
		planets[5].radius = rand_FloatRange(30, 90);
		planets[6].radius = 0;



		transform[23].SetPosition(Vector3(50, 0, -50));
		transform[23].SetEulerRotate(Vector3(0, 0, 0));
		transform[23].SetScale(Vector3(20, 1, 20));

		for (size_t t = 24; t < 30; t++) {
			transform[t].SetEulerRotate(Vector3(0, 90, 0));
			transform[t].SetScale(Vector3(0.5f, 0.5f, 0.5f));
		}



		transform[24].SetPosition(Vector3(50, 0, 0));
		transform[25].SetPosition(Vector3(25, 0, 25));
		transform[26].SetPosition(Vector3 ( -30, 0, -30));
		transform[27].SetPosition(Vector3(25, 0, -40));
		transform[28].SetPosition(Vector3(-50, 0, 10));
		transform[29].SetPosition(Vector3(30, 0, -15));
		BoundingSphere center;
		center.Center = transform[24].GetWorldPosition() +Vector3(0,10,0);
		center.Radius = gameSize/2;
		foodSpheres.push_back(center);
		center.Center = transform[25].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);
		center.Center = transform[26].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);
		center.Center = transform[27].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);
		center.Center = transform[28].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);
		center.Center = transform[29].GetWorldPosition() + Vector3(0, 10, 0);
		center.Radius = gameSize / 2;
		foodSpheres.push_back(center);

 

		playerTransform.SetPosition(Vector3(0, gameSize, 0));
		playerTransform.SetScale(Vector3(gameSize));
		playerTransform.SetEulerRotate(Vector3(0, 0, 0));


		PlayerCollision.Center = playerTransform.GetWorldPosition();
		PlayerCollision.Radius = playerTransform.GetScale().x;

		


	}

