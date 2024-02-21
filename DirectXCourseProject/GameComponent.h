#pragma once
#include "Exports.h"

namespace Engine {
	class Application;
}

class GameComponent
{
protected:
	Engine::Application* _app;
public:
	GameComponent(Engine::Application *app) : _app(app) 
	{

	};
	GameComponent& operator= (const GameComponent &){}
	~GameComponent();
	void DestoyResources();
	void Draw();
	bool Initialize();
	void Reload();
	void Update();
};

