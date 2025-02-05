#pragma once

#ifdef ENGINE_DLL
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif


namespace Engine
{


	class ENGINE_API GameplayInterop
	{
	public:
		static void Init();
	};

}
