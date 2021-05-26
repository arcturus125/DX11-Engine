#include "pch.h"
#include "InitDirect3D.h"
#include <D3D11.h>
#include "Common.h"

HINSTANCE gAppHInstance; // application instance handle
HWND	  gHMainWindow;	 // main window handle
bool	  gAppPaused;    // is the application paused?
bool	  gMinimized;    // is the application minimized?
bool	  gMaximized;    // is the application maximized?
bool	  gResizing;     // are the resize bard being dragged?
UINT	  g4xMsaaQuality;  // quality level of 4x MSAA

ID3D11Device*           gd3dDevice;			  // (4.2.1)
ID3D11DeviceContext*    gd3dImmediateContext; // 
IDXGISwapChain*         gSwapChain;			  // (4.2.4)
ID3D11Texture2D*        gDepthStencilBuffer;  // (4.2.6)
ID3D11RenderTargetView* gRenderTargetView;	  // (4.2.5)
ID3D11DepthStencilView* gDepthStencilView;	  // (4.2.6)
D3D11_VIEWPORT			gScreenViewport;	  // (4.2.8)


// create the d3dDevice and d3dContext
// return true on success
bool CreateDeviceAndContext()
{
	HRESULT hr;

	UINT flags = 0;

    // runs if and ONLY if in DEBUG mode
    #if defined(DEBUG)||defined(_DEBUG)
            // if in debug mode, Set the flags to D3D11_CREATE_DEVICE_DEBUG 
            // to get more debugging information (in the "Output" window of Visual Studio)
            flags = D3D11_CREATE_DEVICE_DEBUG; 
    #endif
	
	hr = D3D11CreateDevice(
		nullptr,								// specify null because we want to use the primary display adapter
		D3D_DRIVER_TYPE_HARDWARE,				// specify 'D3D_DRIVER_TYPE_HARDWARE' to use hardware acelleration
		0,										// speficy null because we are using the hardware for rendering
		flags,									// use flags above
		0,										// specify null to use the greated supported feature level
		0,										// =="==
		D3D11_SDK_VERSION,						// use the DX11 SDK
		&gd3dDevice,							// return the d3dDevice
		nullptr,								// specify null for the greatest feature level
		&gd3dImmediateContext					// return the d3dDeviceContext
		);

	// if the above function was unsuccessful, return an error message to the engine
	if (FAILED(hr))
	{
		gLastError = "Error creating Direct3D device";
		return false;
	}

	gLastError = "Error creating Direct3D device";
	return true;
}
