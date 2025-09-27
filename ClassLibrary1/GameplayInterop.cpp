#include "pch.h"
#include "GameplayInterop.h"
#include "ClassLibrary1.h"
using namespace Engine;
void GameplayInterop::Init()
{
	Class1::Test();
}

void Engine::GameplayInterop::Init(void* a, void* b, void* c)
{
	Class1::Init((IntPtr)a, (IntPtr)b, (IntPtr)c);
}
