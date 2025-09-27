#pragma once

using namespace System;

namespace Engine {
	public ref class Class1
	{
	public:

		static void Test() { 
			Gameplay::SharpClass::Output(); 
		}
		static void Init(IntPtr a, IntPtr b, IntPtr c) {
			Gameplay::SharpClass::Init(a,b,c);
		}
		// TODO: Добавьте сюда свои методы для этого класса.
	};
}
