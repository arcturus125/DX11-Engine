#include "pch.h"
#include "StrangeEngine.h"

std::string gLastError = "no error set";
GameTimer gTimer;



STRANGEENGINEMK3_API void StrangeEngine::StartEngine()
{
	std::cout << "StrangeEngineMK3 starting up\n";
	// Direct X 11 initialization
	DirectX = new InitDirect3D(GetModuleHandle(0));
	DirectX->InitMainWindow();
	if (!DirectX->CreateDeviceAndContext())
	{
		MessageBoxA(DirectX->mHMainWindow, gLastError.c_str(), NULL, MB_OK);
		return;
	}
	DirectX->Check4xMSAAQualitySupport();
	if (!DirectX->DescribeSwapChain())
	{
		MessageBoxA(DirectX->mHMainWindow, gLastError.c_str(), NULL, MB_OK);
		return;
	}
	DirectX->CreateRenderTargetView();
	if (!DirectX->CreateDepthBuffer())
	{
		MessageBoxA(DirectX->mHMainWindow, gLastError.c_str(), NULL, MB_OK);
		return;
	}
	DirectX->BindViewsToOutputMergerStage();
	DirectX->SetViewport();

	std::cout << "StrangeEngineMK3 startup complete\n";

	// runtime
	Run();

}

int StrangeEngine::Run()
{
	MSG msg = { 0 };

	gTimer.Reset();

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			gTimer.Tick();

			if (!DirectX->mAppPaused)
			{
				DirectX->CalculateFrameStats();
				//UpdateScene(mTimer.DeltaTime());
				DirectX->DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}
