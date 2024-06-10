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
#include <src/DebugRenderSysImpl.h>
#include <src/ModelLoader.h>
#include <src/GBuffer.h>
#include <src/ShaderManager.h>
#include <cstdlib>
#include <ctime>
#include <math.h>

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <src/LightComponent.h>




class ENGINE_API Engine::Application;

class GameApplication: public Engine::Application
	{
	public:

		GameApplication();
		~GameApplication();


	
		DebugRenderSysImpl* system;
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
		void ResetGame();


		bool isClosed = false;


		float angle = 0;

		aiNode* node;
		Assimp::Importer importer;

		ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;


		Microsoft::WRL::ComPtr<ID3D11SamplerState> depthSamplerState_ = nullptr;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> defaultDepthState_;

		Transform transform[8];

		float rand_FloatRange(float a, float b)
		{
			return ((b - a) * ((float)rand() / RAND_MAX)) + a;
		}



		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> quadDepthState_;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> rastState_;
		Microsoft::WRL::ComPtr<ID3D11BlendState> blendState_;


		Transform lightTransform;

		Transform playerTransform;
		SphereComponent* player;
		BoundingSphere PlayerCollision{};

		std::vector<std::pair<int, Vector3>> collected;


		Vector3 lightPos;
		Matrix directional_light_projection_;
		Matrix directional_light_view;

		ID3D11SamplerState* sample_state_clamp_ = nullptr;
		ID3D11SamplerState* sample_state_wrap_ = nullptr; 

		ID3D11DepthStencilView* depth_stencil_view_ = nullptr;
		ID3D11RenderTargetView* render_target_view_depth_directional_light_ = nullptr;
		ID3D11ShaderResourceView* resource_view_depth_directional_light_ = nullptr;
		ID3D11ShaderResourceView* depth_shader_resource_view = nullptr;

		std::vector<BoundingSphere> foodSpheres;

		ID3D11SamplerState* TexSamplerState = nullptr;

		const Vector3 offset = { 0,0,-25 };

		float gameSize = 10.0f;

	};
