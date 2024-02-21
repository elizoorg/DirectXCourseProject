#pragma once
#include "Exports.h"
#include "InputDevice.h"
#include "Delegates.h"
#include <iostream>

class WinApi_Display
{
public:
	WinApi_Display();
	~WinApi_Display();
	
	bool CreateDisplay();
	void ShowClientWindow();
	void OnChangeScreenSize();
	bool PollMessages();
	void SetRawInputDevice();
	int getWidth() { return screenWidth; }
	int getHeight() { return screenHeight; }
	HWND getHWND() { return hWnd; }

	int screenWidth = 800;
	int screenHeight = 800;

	DECLARE_MULTICAST_DELEGATE(OnFocusChangedDelegate, bool);
	OnFocusChangedDelegate OnFocusChanged;

	DECLARE_MULTICAST_DELEGATE(OnMouseMoveDelegate, int, int);
	OnMouseMoveDelegate OnMouseMove;
private:
	HWND hWnd;
	LPCWSTR applicationName;
	WNDCLASSEX wc;
	HINSTANCE hInstance;
	RECT windowRect;
};

