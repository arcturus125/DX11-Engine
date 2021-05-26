#pragma once

#include <d3d11.h>
#include "GameTimer.h"
#include <string>

GameTimer mTimer;

extern std::string gLastError;

extern ID3D11Device*			gd3dDevice;			  // (4.2.1)
extern ID3D11DeviceContext*		gd3dImmediateContext; // 
extern IDXGISwapChain*			gSwapChain;			  // (4.2.4)
extern ID3D11Texture2D*			gDepthStencilBuffer;  // (4.2.6)
extern ID3D11RenderTargetView*	gRenderTargetView;	  // (4.2.5)
extern ID3D11DepthStencilView*  gDepthStencilView;	  // (4.2.6)
extern D3D11_VIEWPORT			gScreenViewport;	  // (4.2.8)