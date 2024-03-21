#pragma once
#include "Exports.h"
#include "Display.h"
#include "InputDevice.h"
#include "TriangleComponent.h"
#include "GameComponent.h"
#include "Camera.h"
#include "MathTypes.h"
#include <cstdlib>
#include <ctime>
namespace Engine{

	class ENGINE_API Application
	{
	public:

		
		Application();
		~Application();


		static Application* instance;
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


		bool intersect(Vector2 min_a, Vector2 max_a, Vector2 min_b, Vector2 max_b)
		{
			return (min_a.x <= max_b.x) &&
				(max_a.x >= min_b.x) &&
				(min_a.y <= max_b.y) &&
				(max_a.y >= min_b.y);
		}
		bool isClosed = false;

		int player1_score = 0;
		int player2_score = 0;
		void ResetGame();

		Transform transform[6];

		Microsoft::WRL::ComPtr<ID3D11Device> getDevice() { return device; };
		ID3D11DeviceContext* getContext() { return context; };



		WinApi_Display* getDisplay() { return _display; };
		InputDevice* getInput() { return Device; }

		std::chrono::time_point<std::chrono::steady_clock> PrevTime = std::chrono::steady_clock::now();
		float	deltaTime;
	private:

		std::vector<GameComponent*> Components;

		WinApi_Display* _display;
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

		

		Camera *camera;
		InputDevice *Device;


		
		float totalTime = 0;
		unsigned int frameCount = 0;


	};
}