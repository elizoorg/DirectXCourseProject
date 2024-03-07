#pragma once
#include "Exports.h"
#include "../external/SimpleMath.h"
#include "MathTypes.h"
namespace Engine {
	class Application;
}

class GameComponent
{
protected:
	friend class Engine::Application;
	Engine::Application* _app;
public:
	GameComponent(Engine::Application* app) : _app(app)
	{

	};
	GameComponent& operator= (const GameComponent&) {}
	~GameComponent();
	virtual void DestroyResources() = 0;
	virtual void Draw() = 0;
	virtual bool Initialize() = 0;
	virtual void Reload() = 0;
	virtual void Update() = 0;
	virtual void Update(DirectX::SimpleMath::Matrix mat, Vector4 offset, Vector4 scale,Matrix rotation) = 0;
};