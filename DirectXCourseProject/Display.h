#pragma once
#include "windows.h"
#include "InputDevice.h"
#include <iostream>
class Display
{
public:
	Display();
	~Display();
	HWND hWnd;
	bool CreateDisplay(InputDevice* iDev);
	void ShowClientWindow(InputDevice* iDev);
	void OnChangeScreenSize(const ScreenSize& args);

	int screenWidth = 800;
	int screenHeight = 800;


	LPCWSTR applicationName;
	WNDCLASSEX wc;
	HINSTANCE hInstance;
	RECT windowRect;
};

