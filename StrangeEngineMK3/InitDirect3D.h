#pragma once
#include <d3d11.h>
#include <dxerr.h>
#include <D3D11.h>
#include "Common.h"
#include <iostream>
#include <windowsx.h>
#include <sstream>
#include <xnamath.h>
#include "StrangeEngine.h"

// message handler for windows
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

class StrangeEngine;

class InitDirect3D
{
public:
	static InitDirect3D* singleton;
	StrangeEngine* parentEngine;


	HINSTANCE mAppHInstance; // application instance handle
	HWND	  mHMainWindow;	 // main window handle
	bool	  mAppPaused;    // is the application paused?
	bool	  mMinimized;    // is the application minimized?
	bool	  mMaximized;    // is the application maximized?
	bool	  mResizing;     // are the resize bard being dragged?
	UINT	  m4xMsaaQuality;  // quality level of 4x MSAA
	bool	  mEnable4xMsaa;

	int				mViewportWidth;  // the width of the window
	int				mViewportHeight; // the height of the window
	std::wstring	mMainWndCaption; // the title of the window
	D3D_DRIVER_TYPE md3dDriverType;  // the type of driver, default 'D3D_DRIVER_TYPE_HARDWARE'



	ID3D11Device*			md3dDevice;			  // (4.2.1)
	ID3D11DeviceContext*	md3dImmediateContext; // 
	IDXGISwapChain*			mSwapChain;			  // (4.2.4)
	ID3D11Texture2D*		mDepthStencilBuffer;  // (4.2.6)
	ID3D11RenderTargetView* mRenderTargetView;	  // (4.2.5)
	ID3D11DepthStencilView* mDepthStencilView;	  // (4.2.6)
	D3D11_VIEWPORT			mScreenViewport;	  // (4.2.8)





	// message handlers for my engine
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// init functions
	InitDirect3D(HINSTANCE hInstance, StrangeEngine* strangeEngine_Instance);
	~InitDirect3D();
	bool InitMainWindow();
	bool CreateDeviceAndContext();
	bool Check4xMSAAQualitySupport();
	bool DescribeSwapChain();
	void CreateRenderTargetView();
	bool CreateDepthBuffer();
	void BindViewsToOutputMergerStage();
	void SetViewport();


	// run-time functions

	int Run();
	void CalculateFrameStats();
	void DrawScene();


	void OnResize();
};


