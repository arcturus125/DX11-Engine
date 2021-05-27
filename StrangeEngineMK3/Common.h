#pragma once

#include <d3d11.h>
#include "GameTimer.h"
#include <string>

extern GameTimer gTimer;

extern std::string gLastError;

extern HINSTANCE gAppHInstance; // application instance handle
extern HWND	  gHMainWindow;	 // main window handle
extern bool	  gAppPaused;    // is the application paused?
extern bool	  gMinimized;    // is the application minimized?
extern bool	  gMaximized;    // is the application maximized?
extern bool	  gResizing;     // are the resize bard being dragged?
extern UINT	  g4xMsaaQuality;  // quality level of 4x MSAA

extern int gViewportWidth;	 // the width and height of the window
extern int gViewportHeight;	 //

extern ID3D11Device*			gd3dDevice;			  // (4.2.1)
extern ID3D11DeviceContext*		gd3dImmediateContext; // 
extern IDXGISwapChain*			gSwapChain;			  // (4.2.4)
extern ID3D11Texture2D*			gDepthStencilBuffer;  // (4.2.6)
extern ID3D11RenderTargetView*	gRenderTargetView;	  // (4.2.5)
extern ID3D11DepthStencilView*  gDepthStencilView;	  // (4.2.6)
extern D3D11_VIEWPORT			gScreenViewport;	  // (4.2.8)