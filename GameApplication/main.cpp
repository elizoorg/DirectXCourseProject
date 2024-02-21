// MySuper3DApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <WinUser.h>
#include <wrl.h>
#include <iostream>
#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <chrono>



#include <Application.h>

int main()
{
	Engine::Application::Instance().Run();
}
