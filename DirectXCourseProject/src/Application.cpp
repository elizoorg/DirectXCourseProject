#include "Application.h"
namespace Engine {
	Application& Application::Instance()
	{
		static Application a{};
		return a;
		// TODO: insert return statement here
	}
	Application::Application()
	{
		std::cout << "It's a Engine time\n";
	}

	Application::~Application()
	{
	}


	int Application::Run() {
		
		_display.CreateDisplay();
		//_display.OnMouseMove += [](InputDevice::RawMouseEventArgs args) {InputDevice::Instance().OnMouseMove(args); };
		//_display.OnKeyDown += [](InputDevice::KeyboardInputEventArgs args) {InputDevice::Instance().OnKeyDown(args); };


		Initialize();

		while (true) {

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



		//std::cout << camera.GetPosition().x << " " << camera.GetPosition().y << " " << camera.GetPosition().z << std::endl;

		context->OMSetRenderTargets(1, &rtv, depthStencilView.Get());
		


		float color[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		context->ClearRenderTargetView(rtv, color);
		context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,1.0f,0);
		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(_display.getWidth());
		viewport.Height = static_cast<float>(_display.getHeight());
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
		camera.SetPosition(0, 0, 0);
		swapDesc.BufferCount = 2;
		swapDesc.BufferDesc.Width = _display.getWidth();
		swapDesc.BufferDesc.Height = _display.getHeight();
		swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapDesc.OutputWindow = _display.getHWND();
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
		res = device->CreateRenderTargetView(backTex, nullptr, &rtv);
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens2\n";
		}

		TriangleComponent* trag = new TriangleComponent(&Application::Instance());
		TriangleComponent* trag1 = new TriangleComponent(&Application::Instance());
		Components.push_back(trag);
		Components.push_back(trag1);

		for (auto comp : Components) {
			comp->Initialize();
		}



		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FillMode = D3D11_FILL_WIREFRAME;





		res = device->CreateRasterizerState(&rastDesc, &rastState);


		D3D11_TEXTURE2D_DESC depthStencilDesc;
		depthStencilDesc.Width= _display.getWidth();
		depthStencilDesc.Height= _display.getHeight();
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
			std::cout << "So,unexpected shit happens2\n";
		}

		res = device->CreateDepthStencilView(depthStencilBuffer.Get(), NULL, depthStencilView.GetAddressOf());
		if (FAILED(res)) {
			std::cout << "So,unexpected shit happens2\n";
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
		while (!_display.PollMessages()) {
			//std::cout << InputDevice::Instance().getMousePos().x << InputDevice::Instance().getMousePos().y << std::endl;
			//std::cout << "We are running!\n";
		}

		if(InputDevice::Instance().IsKeyDown(Keys::D))
		{
			camera.Strafe(1);
		}
		if (InputDevice::Instance().IsKeyDown(Keys::A))
		{
			camera.Strafe(-1);
		}
		if (InputDevice::Instance().IsKeyDown(Keys::W))
		{
			camera.Walk(1);
		}
		if (InputDevice::Instance().IsKeyDown(Keys::S))
		{
			camera.Walk(-1);
		}


		
		//camera.Pitch(0.01);
		//camera.RotateY(0.01);
		//camera.Strafe(1);
		camera.UpdateViewMatrix();
		Matrix matrix;
		for (auto comp : Components) {
			comp->Update(camera.ViewProj());
		}


		auto	curTime = std::chrono::steady_clock::now();
		float	deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(curTime - PrevTime).count() / 1000000.0f;
		PrevTime = curTime;

		totalTime += deltaTime;
		frameCount++;

		return true;
	}

	void Application::UpdateInternal()
	{
	}

}