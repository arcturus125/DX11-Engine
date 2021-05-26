#include "pch.h"
#include "StrangeEngine.h"
#include <iostream>
#include "InitDirect3D.h"

std::string gLastError = "no error set";

STRANGEENGINEMK3_API void StrangeEngine::StartEngine()
{
	std::cout << "StrangeEngineMK3 starting up\n";
	CreateDeviceAndContext();

	std::cout << "last error: " << gLastError;
}
