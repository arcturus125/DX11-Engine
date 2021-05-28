#pragma once

#include "Common.h"
#include <iostream>
#include "InitDirect3D.h"

#ifdef STRANGEENGINEMK3_EXPORTS
#define STRANGEENGINEMK3_API __declspec(dllexport)
#else
#define STRANGEENGINEMK3_API __declspec(dllimport)
#endif // STRANGEENGINE_EXPORTS
class InitDirect3D;

class StrangeEngine
{
public:
	InitDirect3D* DirectX;

	STRANGEENGINEMK3_API void StartEngine();
	int Run();
};

