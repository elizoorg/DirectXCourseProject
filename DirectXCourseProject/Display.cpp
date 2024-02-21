#include "pch.h"
#include "Exports.h"
#include "Display.h"




LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	switch (umessage)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
	{
		// If a key is pressed send it to the input object so it can record that state.
		std::cout << "Key: " << static_cast<unsigned int>(wparam) << std::endl;

		if (static_cast<unsigned int>(wparam) == 27) PostQuitMessage(0);
		return 0;
	}
	default:
	{
		return DefWindowProc(hwnd, umessage, wparam, lparam);
	}
	}
}


WinApi_Display::WinApi_Display()
{
}

WinApi_Display::~WinApi_Display()
{
}


void WinApi_Display::OnChangeScreenSize() {
	//screenWidth = args.Width;
	//creenHeight = args.Height;
}

bool WinApi_Display::PollMessages()
{
	//std::cout << "StartPollMessages\n";
	MSG msg = {};
	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// If windows signals to end the application then exit out.
	if (msg.message == WM_QUIT) {
		return false;
	}

	POINT p;
	GetCursorPos(&p);
	ScreenToClient(hWnd, &p);
	OnMouseMove.Broadcast(p.x, p.y);
	//std::cout << "EndPollMessages\n";
	return true;
}

void WinApi_Display::SetRawInputDevice()
{
	RAWINPUTDEVICE Rid[2];

	Rid[0].usUsagePage = 0x01;
	Rid[0].usUsage = 0x02;
	Rid[0].dwFlags = 0;   // adds HID mouse and also ignores legacy mouse messages
	Rid[0].hwndTarget = hWnd;

	Rid[1].usUsagePage = 0x01;
	Rid[1].usUsage = 0x06;
	Rid[1].dwFlags = 0;   // adds HID keyboard and also ignores legacy keyboard messages
	Rid[1].hwndTarget = hWnd;

	if (RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])) == FALSE)
	{
		auto errorCode = GetLastError();
		std::cout << "ERROR: " << errorCode << std::endl;
	}
}

bool WinApi_Display::CreateDisplay()
{
	applicationName = L"My3DApp";
	hInstance = GetModuleHandle(nullptr);


	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);


	windowRect = { 0, 0, static_cast<LONG>(screenWidth), static_cast<LONG>(screenHeight) };

	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	auto dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME;

	auto posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
	auto posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;

	hWnd = CreateWindowEx(WS_EX_APPWINDOW, applicationName, applicationName,
		dwStyle,
		posX, posY,
		windowRect.right - windowRect.left,
		windowRect.bottom - windowRect.top,
		nullptr, nullptr, hInstance, nullptr);
	if (!hWnd) {
		std::cout << "Error while creating window!\n";
		__debugbreak();
	}
	SetRawInputDevice();
	ShowClientWindow();
	
	


	return false;
}

void WinApi_Display::ShowClientWindow()
{
	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);
	ShowCursor(true);
	UpdateWindow(hWnd);

}

