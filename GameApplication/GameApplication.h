#pragma once
#include <src/Exports.h>
#include <src/Application.h>
#include <src/Display.h>
#include <src/InputDevice.h>
#include <src/TriangleComponent.h>
#include <src/GameComponent.h>
#include <src/Camera.h>
#include <src/MathTypes.h>
#include <src/SphereComponent.h>
#include <cstdlib>
#include <ctime>
#include <math.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

class ENGINE_API Engine::Application;

class GameApplication: public Engine::Application
	{
	public:

		GameApplication();
		~GameApplication();



		static GameApplication* instance;
		int Run() override;
		void CreateBackBuffer() override;
		void DestroyResources() override;
		void Draw() override;
		void EndFrame() override;
		void Exit() override;
		void Initialize() override;
		void MessageHandler() override;
		void PrepareFrame() override;
		void PrepareRecources() override;
		void RestoreTargets() override;
		bool Update() override;
		void UpdateInternal() override;



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

		float angle = 0;


		ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

		Transform transform[22];

		float rand_FloatRange(float a, float b)
		{
			return ((b - a) * ((float)rand() / RAND_MAX)) + a;
		}

		struct PlanetTransform {
			Vector3 joint = Vector3(0,0,0);
			float radius;
			float angle=5.0f;
			float angle2=5.0f;
			float angleSpeed;
			float angleSpeed2;
		} planets[22];


	};
