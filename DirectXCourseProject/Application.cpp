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
		_display.OnMouseMove += [](InputDevice::RawMouseEventArgs args) {InputDevice::Instance().OnMouseMove(args); };
		_display.OnKeyDown += [](InputDevice::KeyboardInputEventArgs args) {InputDevice::Instance().OnKeyDown(args); };

		std::cout << "Draw\n";
		Initialize();
		std::cout << "Draw\n";
		while (true) {
			std::cout << "Draw\n";
			Update();
			Draw();
		};

		std::cout << "Draw\n";
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
		std::cout << "Run Draw\n";
		context->ClearState();

		context->RSSetState(rastState);

		D3D11_VIEWPORT viewport = {};
		viewport.Width = static_cast<float>(_display.getWidth());
		viewport.Height = static_cast<float>(_display.getHeight());
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1.0f;

		context->RSSetViewports(1, &viewport);


		context->OMSetRenderTargets(1, &rtv, nullptr);

		float color[] = { totalTime, 0.1f, 0.1f, 1.0f };
		context->ClearRenderTargetView(rtv, color);


		for (auto comp : Components)
			comp->Draw();

		context->OMSetRenderTargets(0, nullptr, nullptr);

		swapChain->Present(1, /*DXGI_PRESENT_DO_NOT_WAIT*/ 0);

		std::cout << "End Draw\n";


	}

	void Application::EndFrame()
	{
	}

	void Application::Exit()
	{
	}

	void Application::Initialize()
	{

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


		Components.push_back(new TriangleComponent(&Application::Instance()));
		for(auto comp :Components){
			comp->Initialize();
		}

		res = device->CreateRasterizerState(&rastDesc, &rastState);

		rastDesc.CullMode = D3D11_CULL_NONE;
		rastDesc.FillMode = D3D11_FILL_SOLID;

		context->RSSetState(rastState);
	
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