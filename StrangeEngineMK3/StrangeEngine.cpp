#include "pch.h"
#include "StrangeEngine.h"
#include <iostream>
#include "InitDirect3D.h"
#include "Common.h"

std::string gLastError = "no error set";
GameTimer gTimer;

STRANGEENGINEMK3_API void StrangeEngine::StartEngine()
{
	std::cout << "StrangeEngineMK3 starting up\n";
	SetDefaults(GetModuleHandle(0));
	InitMainWindow();
	if (!CreateDeviceAndContext())
	{
		MessageBoxA(gHMainWindow, gLastError.c_str(), NULL, MB_OK);
		return;
	}
	Check4xMSAAQualitySupport();
	if (!DescribeSwapChain())
	{
		MessageBoxA(gHMainWindow, gLastError.c_str(), NULL, MB_OK);
		return;
	}

}
