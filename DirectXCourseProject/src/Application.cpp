#include "Application.h"

#include "SphereComponent.h"

namespace Engine {
	
	Application::Application()
	{

		instance = this;
		Device = new InputDevice(this);
		std::cout << "It's a Engine time\n";
	}
	Application* Application::instance = nullptr;
	Application::~Application()
	{
	}


	int Application::Run() {

		_display = new WinApi_Display(this);
		_display->CreateDisplay();
		//_display.OnMouseMove += [](InputDevice::RawMouseEventArgs args) {InputDevice::Instance().OnMouseMove(args); };
		//_display.OnKeyDown += [](InputDevice::KeyboardInputEventArgs args) {InputDevice::Instance().OnKeyDown(args); };


		Initialize();

		while (!isClosed) {
			UpdateInternal();
			Update();
			Draw();
		};


		return 0;
		
	}

	void Application::CreateBackBuffer()
	{
	}

	void Application::DestroyResources()
	{
	}

	void Application::Draw()
	{

		context->OMSetRenderTargets(1, &rtv, depthStencilView.Get());
		


		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		context->ClearRenderTargetView(rtv, color);
		context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,1.0f,0);
		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(_display->getWidth());
		viewport.Height = static_cast<float>(_display->getHeight());
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1.0f;

		context->RSSetViewports(1, &viewport);


	

		for (auto& comp : Components)
			comp->Draw();


		context->OMSetRenderTargets(0, nullptr, nullptr);

		swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);

		


	}

	void Application::EndFrame()
	{
	}

	void Application::Exit()
	{
	}

	void Application::Initialize()
	{
		Device = new InputDevice(this);
		camera = new Camera(this);
		camera->SetPosition(0, 0, 200);
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

		TriangleComponent* trag = new TriangleComponent(this);
		TriangleComponent* trag1 = new TriangleComponent(this);
		TriangleComponent* trag2 = new TriangleComponent(this);
		SphereComponent* sphere = new SphereComponent(this);
		SphereComponent* sphere2 = new SphereComponent(this);
		SphereComponent* sphere3 = new SphereComponent(this);
		Components.push_back(trag);
		Components.push_back(trag1);
		Components.push_back(trag2);
		Components.push_back(sphere);
		Components.push_back(sphere2);
		Components.push_back(sphere3);

		for (auto comp : Components) {
			comp->Initialize();
		}



		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FillMode = D3D11_FILL_WIREFRAME;


		std::srand(std::time(nullptr));

		ResetGame();


		res = device->CreateRasterizerState(&rastDesc, &rastState);
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens4\n";
		}

		D3D11_TEXTURE2D_DESC depthStencilDesc;
		depthStencilDesc.Width= _display->getWidth();
		depthStencilDesc.Height= _display->getHeight();
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		res = device->CreateTexture2D(&depthStencilDesc, NULL, depthStencilBuffer.GetAddressOf());
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens5\n";
		}

		res = device->CreateDepthStencilView(depthStencilBuffer.Get(), NULL, depthStencilView.GetAddressOf());
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens6\n";
		}




	
	}

	void Application::MessageHandler()
	{
	}

	void Application::PrepareFrame()
	{
	}

	void Application::PrepareRecources()
	{
	}

	void Application::RestoreTargets()
	{
	}

	bool Application::Update()
	{
		
		while(!_display->PollMessages())
		{
			
		}
		
		if (Device->IsKeyDown(Keys::Escape))
		{
			isClosed = true;
			return true;
		}
		if (Device->IsKeyDown(Keys::D))
		{
			camera->Strafe(1);
		}
		if (Device->IsKeyDown(Keys::A))
		{
			camera->Strafe(-1);
		}
		if (Device->IsKeyDown(Keys::S))
		{
			camera->Walk(1);
		}
		if (Device->IsKeyDown(Keys::W))
		{
			camera->Walk(-1);
		}
		if (Device->IsKeyDown(Keys::Space))
		{
			camera->Fly(1);
		}
		if (Device->IsKeyDown(Keys::LeftShift))
		{
			camera->Fly(-1);
		}
		if (Device->IsKeyDown(Keys::U))
		{
			players[0].offset.y += 2;
		}
		if (Device->IsKeyDown(Keys::J))
		{
			players[0].offset.y -= 2;
		}
		if (Device->IsKeyDown(Keys::NumPad2))
		{
			players[1].offset.y -= 2;
		}
		if (Device->IsKeyDown(Keys::NumPad8))
		{
			players[1].offset.y += 2;
		}
		
		if (Device->IsKeyDown(Keys::LeftControl) || Device->IsKeyDown(Keys::RightAlt) ) {
			std::cout << "It's working!\n";
		}


		players[2].offset.x += players[2].speed.x;
		players[2].offset.y += players[2].speed.y;

		if (players[2].offset.y > 100 || players[2].offset.y < -100) players[2].speed.y *= -1;

		if (players[2].offset.x + 6 > players[1].offset.x)
		{
			if (intersect(Vector2(players[2].offset.x - 3, players[2].offset.y - 3),
				Vector2(players[2].offset.x + 3, players[2].offset.y + 3),
				Vector2(players[1].offset.x - 3, players[1].offset.y - 15),
				Vector2(players[1].offset.x + 3, players[1].offset.y + 15))
				)
			{
				players[2].speed.x *= -1.5;
			}
			else
			{
				player1_score++;
				std::cout << "Player 1 :" << player1_score << " Player2 : " << player2_score << std::endl;
				//ResetGame();
			}
		}

		if (players[2].offset.x - 6 < players[0].offset.x)
		{
			if (intersect(
				Vector2(players[0].offset.x - 3, players[0].offset.y - 15),
				Vector2(players[0].offset.x + 3, players[0].offset.y + 15),
				Vector2(players[2].offset.x - 3, players[2].offset.y - 3),
				Vector2(players[2].offset.x + 3, players[2].offset.y + 3)
			))

			{
				players[2].speed.x *= -1.5;
			}
			else
			{
				player2_score++;
				std::cout << "Player 1 :" << player1_score << " Player2 : " << player2_score << std::endl;
				//ResetGame();
			}
		}

		if (players[2].speed.x > 0.6) players[2].speed.x = 0.6;
		if (players[2].speed.x < -0.6) players[2].speed.x = -0.6;


		players[3].angle += 1;
		players[4].angle -= 1;
		players[5].angle += 5;

		
		players[4].offset = players[3].offset + Vector4::Transform(Vector4(50, 0, 0, 1.0f), Matrix::CreateFromAxisAngle(players[3].rotation,
			Math::Radians(players[3].angle)));


		//camera.Pitch(0.01);
		//camera.RotateY(0.01);
		//camera.Strafe(1);
		camera->UpdateViewMatrix();
		//Vector3 scale(5.0f, 1.0f, 1.0f);
		//Vector3 offset(1.0f, 1.0f, 1.0f);


			for (size_t t = 0; t < Components.size(); t++) {
				Components[t]->Update(camera->ViewProj(), players[t].offset, players[t].scale,
					Matrix::CreateFromAxisAngle(players[t].rotation, Math::Radians(players[t].angle)));
			}




			return true;
		
	}

		void Application::UpdateInternal()
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
		void Application::ResetGame()
		{
			players[0].offset = Vector4(-100, 0, 0, 1.0f);
			players[1].offset = Vector4(100, 0, 0, 1.0f);

			players[0].scale = Vector4(1, 10, 1, 1);
			players[1].scale = Vector4(1, 10, 1, 1);
			players[2].scale = Vector4(1.0f, 1.0f, 1.0f, 1);
			players[2].offset = Vector4(0, 0, 0, 1.0f);
			players[2].speed.x = ((double)std::rand() / (RAND_MAX + 1)) * 2;
			players[2].speed.y = ((double)std::rand() / (RAND_MAX + 1)) * 2;


			players[3].offset = Vector4(0, 0, 0, 1.0f);
			players[3].scale = Vector4(10.0f, 10.0f, 10.0f, 1.0f);

			players[3].rotation = Vector3(1, 0, 0);
			players[4].rotation = Vector3(((double)rand() / (RAND_MAX)), ((double)rand() / (RAND_MAX)), ((double)rand() / (RAND_MAX)));
			players[5].rotation = Vector3(((double)rand() / (RAND_MAX)), ((double)rand() / (RAND_MAX)), ((double)rand() / (RAND_MAX)));
			
			players[0].rotation = Vector3(0, 0, 0);
			players[0].rotation = Vector3(1, 0, 0);
			players[1].rotation = Vector3(0, 1, 0);
			players[2].rotation = Vector3(0, 0, 1);
			players[0].angle = 0;
			players[1].angle = 0;
			players[2].angle = 0;
			
			
			players[4].offset = Vector4(-40, -40, 0, 1.0f);
			players[4].scale = Vector4(10.0f, 10.0f, 10.0f, 1.0f);
		
			players[5].offset = Vector4(40, 40, 0, 1.0f);
			players[5].scale = Vector4(10.0f, 10.0f, 10.0f, 1.0f);
		}


}
