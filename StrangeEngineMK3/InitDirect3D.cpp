#include "pch.h"
#include "InitDirect3D.h"
#include <D3D11.h>
#include "Common.h"
#include <iostream>
#include <windowsx.h>

HINSTANCE gAppHInstance; // application instance handle
HWND	  gHMainWindow;	 // main window handle
bool	  gAppPaused;    // is the application paused?
bool	  gMinimized;    // is the application minimized?
bool	  gMaximized;    // is the application maximized?
bool	  gResizing;     // are the resize bard being dragged?
UINT	  g4xMsaaQuality;  // quality level of 4x MSAA
bool	  enable4xMsaa;

int gViewportWidth;
int gViewportHeight;
std::wstring mMainWndCaption;
D3D_DRIVER_TYPE md3dDriverType;



ID3D11Device*           gd3dDevice;			  // (4.2.1)
ID3D11DeviceContext*    gd3dImmediateContext; // 
IDXGISwapChain*         gSwapChain;			  // (4.2.4)
ID3D11Texture2D*        gDepthStencilBuffer;  // (4.2.6)
ID3D11RenderTargetView* gRenderTargetView;	  // (4.2.5)
ID3D11DepthStencilView* gDepthStencilView;	  // (4.2.6)
D3D11_VIEWPORT			gScreenViewport;	  // (4.2.8)


// ==============================================================
//		windows message handlers
// ==============================================================

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return MsgProc(hwnd, msg, wParam, lParam);
}
LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			gAppPaused = true;
			gTimer.Stop();
		}
		else
		{
			gAppPaused = false;
			gTimer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		gViewportWidth = LOWORD(lParam);
		gViewportHeight = HIWORD(lParam);
		if (gd3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				gAppPaused = true;
				gMinimized = true;
				gMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				gAppPaused = false;
				gMinimized = false;
				gMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (gMinimized)
				{
					gAppPaused = false;
					gMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (gMaximized)
				{
					gAppPaused = false;
					gMaximized = false;
					OnResize();
				}
				else if (gResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					OnResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		gAppPaused = true;
		gResizing = true;
		gTimer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		gAppPaused = false;
		gResizing = false;
		gTimer.Start();
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// ==============================================================
//		Init functions
// ==============================================================

void SetDefaults(HINSTANCE hInstance)
{
	gAppHInstance = hInstance;
	mMainWndCaption = L"StrangeEngine";

	md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	gViewportWidth = 800;
	gViewportHeight = 600;
	enable4xMsaa = false;
	gHMainWindow = 0;
	gAppPaused = false;
	gMinimized = false;
	gMaximized = false;
	gResizing = false;
	g4xMsaaQuality = 0;

	gd3dDevice = 0;
	gd3dImmediateContext = 0;
	gSwapChain = 0;
	gDepthStencilBuffer = 0;
	gRenderTargetView = 0;
	gDepthStencilView = 0;

	
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "Defaults Set" << std::endl;
	#endif
}

bool InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = gAppHInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, gViewportWidth, gViewportHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	gHMainWindow = CreateWindow(L"D3DWndClassName", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, gAppHInstance, 0);
	if (!gHMainWindow)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(gHMainWindow, SW_SHOW);
	UpdateWindow(gHMainWindow);

	return true;
}

// create the d3dDevice and d3dContext
// return true on success
bool CreateDeviceAndContext()
{
	HRESULT hr;


	// if in debug mode, Set the flags to D3D11_CREATE_DEVICE_DEBUG 
	// to get more debugging information (in the "Output" window of Visual Studio)
	// otherwise leave the default flags
	UINT flags = 0;
    #if defined(DEBUG)||defined(_DEBUG)
            flags = D3D11_CREATE_DEVICE_DEBUG; 
    #endif

	D3D_FEATURE_LEVEL featureLevel;
	
	hr = D3D11CreateDevice(
		nullptr,								// specify null because we want to use the primary display adapter
		D3D_DRIVER_TYPE_HARDWARE,				// specify 'D3D_DRIVER_TYPE_HARDWARE' to use hardware acelleration
		0,										// speficy null because we are using the hardware for rendering
		flags,									// use flags above
		0,										// specify null to use the greated supported feature level
		0,										// =="==
		D3D11_SDK_VERSION,						// use the DX11 SDK
		&gd3dDevice,							// return the d3dDevice
		&featureLevel,							// return the feature level
		&gd3dImmediateContext					// return the d3dDeviceContext
		);

	// if the above function was unsuccessful
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: Error creating Direct3D device" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "Error creating Direct3D device";
		return false;
	}

	// make sure that feature level 11 (directx 11) is supported
	if (featureLevel != D3D_FEATURE_LEVEL_11_0)
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: Direct3D Feature Level 11 Unsupported" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "Direct3D Feature Level 11 Unsupported";
		return false;
	}
	// Debug Logs
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "Direct3D device successfully created!" << std::endl;
	#endif

	return true;
}

bool Check4xMSAAQualitySupport()
{
	HRESULT hr = gd3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &g4xMsaaQuality);

	// if the above function was unsuccessful
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[WARN]: 4x MSAA Unsupported, disabling 4xMSAA" << std::endl;
		#endif

		enable4xMsaa = false;
		return false;
	}

	if (g4xMsaaQuality <= 0)
	{// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[WARN]: 4x MSAA cannot  be negative, disabling 4xMSAA" << std::endl;
		#endif

		enable4xMsaa = false;
		return false;
	}

	
	// Debug logs
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "4x MSAA supported, enabling 4xMSAA" << std::endl;
	#endif
	enable4xMsaa = true;
	return true;
}

bool DescribeSwapChain()
{
	// ==============================================================
	//		describe the swap chain
	// ==============================================================
	
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.OutputWindow = gHMainWindow;                           // set the output windoe of the swap chain to be the window we have already created
    swapChainDesc.Windowed = TRUE;                                // specify TRUE to run inwindowed mode and FALSE to run in fullscreen mode
    swapChainDesc.BufferCount = 1;                                // the number of back buffers we want to use, typically 1 is used fo rback buffering but more can be used
    swapChainDesc.BufferDesc.Width  = gViewportWidth;             // Target window size
    swapChainDesc.BufferDesc.Height = gViewportHeight;            // --"--
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Pixel format of target window
    swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;        // Refresh rate of monitor (provided as fraction = 60/1 here)
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;         // --"--
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // specify how we are going be use the back buffer. because we will be rendering to this back biffer, we specify 'DXGI_USAGE_RENDER_TARGET_OUTPUT'
	// use 4X MSAA
	if (enable4xMsaa)
	{
		swapChainDesc.SampleDesc.Count = 4;
		swapChainDesc.SampleDesc.Quality = g4xMsaaQuality-1;
	}
	// no MSAA
	else
	{
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
	}
	

	// ==============================================================
	//		create the swap chain
	// ==============================================================

	HRESULT hr;

	IDXGIDevice* dxgiDevice;
	hr = gd3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**) &dxgiDevice);
	// check for failure
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: Could not obtain DXGIDevice" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "Could not obtain DXGIDevice";
		dxgiDevice->Release();  dxgiDevice = nullptr;
		return false;
	}

	IDXGIAdapter* dxgiAdapter;
	hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**) &dxgiAdapter);
	// check for failure
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: Could not obtain DXGIAdapter" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "Could not obtain DXGIAdapter";
		dxgiDevice->Release();  dxgiDevice = nullptr;
		dxgiAdapter->Release(); dxgiAdapter = nullptr;
		return false;
	}

	IDXGIFactory* dxgiFactory;
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory);
	// check for failure
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: Could not obtain DXGIFactory" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "Could not obtain DXGIFactory";
		dxgiDevice->Release();  dxgiDevice = nullptr;
		dxgiAdapter->Release(); dxgiAdapter = nullptr;
		dxgiFactory->Release(); dxgiFactory = nullptr;
		return false;
	}


	hr = dxgiFactory->CreateSwapChain(
		gd3dDevice,
		&swapChainDesc,
		&gSwapChain
	);
	// check the swap chain has been made sucessfully
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: Could not create swap chain" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "error creating swap chain";
		dxgiDevice->Release();  dxgiDevice = nullptr;
		dxgiAdapter->Release(); dxgiAdapter = nullptr;
		dxgiFactory->Release(); dxgiFactory = nullptr;
		return false;
	}


	//release memory now we are done with it
	dxgiDevice->Release();  dxgiDevice = nullptr;
	dxgiAdapter->Release(); dxgiAdapter = nullptr;
	dxgiFactory->Release(); dxgiFactory = nullptr;


	
	// Debug logs
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "swap chain successfully created" << std::endl;
	#endif
	return true;
}


// ==============================================================
//	 Run-time functions
// ==============================================================

void OnResize()
{
	if (!(gd3dImmediateContext != nullptr &&
		gd3dDevice != nullptr &&
		gSwapChain != nullptr))
		return;

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	gRenderTargetView->Release(); gRenderTargetView = nullptr;
	gDepthStencilView->Release(); gRenderTargetView = nullptr;
	gDepthStencilBuffer->Release(); gDepthStencilBuffer = nullptr;


	// Resize the swap chain and recreate the render target view.
	HRESULT hr;
	hr = gSwapChain->ResizeBuffers(1, gViewportWidth, gViewportHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	// check that the buffer has successfully been resized
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: error resizing swap chain depth buffers" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "error resizing swap chain depth buffers";
	}

	ID3D11Texture2D* backBuffer;
	hr = gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: error accessing depth buffer after resizing" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "error accessing depth buffer after resizing";
	}

	hr = gd3dDevice->CreateRenderTargetView(backBuffer, 0, &gRenderTargetView);
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: error creating render target view after resizing" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "error creating render target view after resizing";
	}
	backBuffer->Release(); backBuffer = nullptr;

	// Create the depth/stencil buffer and view.

	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = gViewportWidth;
	depthStencilDesc.Height = gViewportHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (enable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = g4xMsaaQuality - 1;
	}
	// No MSAA
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}

	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = gd3dDevice->CreateTexture2D(&depthStencilDesc, 0, &gDepthStencilBuffer);
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: error creating new depth buffer after resizing" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "error creating new depth buffer after resizing";
	}

	hr = gd3dDevice->CreateDepthStencilView(gDepthStencilBuffer, 0, &gDepthStencilView);
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: error creating new depth stencil view after resizing" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "error creating new depth stencil view after resizing";
	}


	// Bind the render target view and depth/stencil view to the pipeline.

	gd3dImmediateContext->OMSetRenderTargets(1, &gRenderTargetView, gDepthStencilView);


	// Set the viewport transform.

	gScreenViewport.TopLeftX = 0;
	gScreenViewport.TopLeftY = 0;
	gScreenViewport.Width = static_cast<float>(gViewportWidth);
	gScreenViewport.Height = static_cast<float>(gViewportHeight);
	gScreenViewport.MinDepth = 0.0f;
	gScreenViewport.MaxDepth = 1.0f;
	
	gd3dImmediateContext->RSSetViewports(1, &gScreenViewport);
}

void OnMouseDown(WPARAM btnState, int x, int y)
{
}

void OnMouseUp(WPARAM btnState, int x, int y)
{
}

void OnMouseMove(WPARAM btnState, int x, int y)
{
}
