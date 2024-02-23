#include "InputDevice.h"

using namespace DirectX::SimpleMath;

InputDevice& InputDevice::Instance()
{
	static InputDevice i{};
	return i;
}


InputDevice::InputDevice()
{
	keys = new std::unordered_set<Keys>();
}

InputDevice::~InputDevice()
{
	delete keys;
}

void InputDevice::OnKeyDown(KeyboardInputEventArgs args)
{
	bool Break = args.Flags & 0x01;

	auto key = static_cast<Keys>(args.VKey);

	if (args.MakeCode == 42) key = Keys::LeftShift;
	if (args.MakeCode == 54) key = Keys::RightShift;
	
	if(Break) {
		if(keys->count(key))	RemovePressedKey(key);
	} else {
		if (!keys->count(key))	AddPressedKey(key);
	}
}



void InputDevice::OnMouseWheel(float mouseWheel)
{
}



void InputDevice::OnMouseKey(int keyCode, bool isDown)
{
	/*if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::LeftButtonDown))
		AddPressedKey(Keys::LeftButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::LeftButtonUp))
		RemovePressedKey(Keys::LeftButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::RightButtonDown))
		AddPressedKey(Keys::RightButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::RightButtonUp))
		RemovePressedKey(Keys::RightButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::MiddleButtonDown))
		AddPressedKey(Keys::MiddleButton);
	if (args.ButtonFlags & static_cast<int>(MouseButtonFlags::MiddleButtonUp))
		RemovePressedKey(Keys::MiddleButton);*/
}

void InputDevice::OnMouseMove(RawMouseEventArgs args)
{

	


	
	MouseWheelDelta = args.WheelDelta;

	const MouseMoveEventArgs moveArgs = {MousePosition, MouseOffset, MouseWheelDelta};

	printf(" Mouse: posX=%04.4f posY:%04.4f offsetX:%04.4f offsetY:%04.4f, wheelDelta=%04d \n",
		MousePosition.x,
		MousePosition.y,
		MouseOffset.x,
		MouseOffset.y,
		MouseWheelDelta);
}

void InputDevice::OnChangeScreenSize(int width, int height) {
	ScreenParam.Width = width;
	ScreenParam.Height = height;
};


void InputDevice::AddPressedKey(Keys key)
{
	//if (!game->isActive) {
	//	return;
	//}
	keys->insert(key);
}

void InputDevice::RemovePressedKey(Keys key)
{
	keys->erase(key);
}

bool InputDevice::IsKeyDown(Keys key)
{
	return keys->count(key);
}

