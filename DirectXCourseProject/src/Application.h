#pragma once
#include "Exports.h"
#include "Display.h"
#include "InputDevice.h"
#include "TriangleComponent.h"
#include "GameComponent.h"
#include "Camera.h"
#include "MathTypes.h"
namespace Engine{

	class ENGINE_API Application
	{
	public:

		static Application& Instance();

		Application();
		~Application();
		int Run();
		void CreateBackBuffer();
		void DestroyResources();
		void Draw();
		void EndFrame();
		void Exit();
		void Initialize();
		void MessageHandler();
		void PrepareFrame();
		void PrepareRecources();
		void RestoreTargets();
		bool Update();
		void UpdateInternal();


		WinApi_Display getDisplay() { return _display; };
		Microsoft::WRL::ComPtr<ID3D11Device> getDevice() { return device; };
		ID3D11DeviceContext* getContext() { return context; };
	private:

		std::vector<GameComponent*> Components;

		WinApi_Display _display;
		DXGI_SWAP_CHAIN_DESC swapDesc;
		Microsoft::WRL::ComPtr<ID3D11Device> device;
		ID3D11DeviceContext* context;
		IDXGISwapChain* swapChain;

		ID3D11Texture2D* backTex;
		ID3D11RenderTargetView* rtv;

		ID3D11RasterizerState* rastState;
		CD3D11_RASTERIZER_DESC rastDesc = {};

		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;

		

		Camera camera;


		std::chrono::time_point<std::chrono::steady_clock> PrevTime = std::chrono::steady_clock::now();
		float totalTime = 0;
		unsigned int frameCount = 0;


	};
}