#include "pch.h"
#include "InitDirect3D.h"

// gives the singleton an initial value to clear up any unresolved externals
InitDirect3D* InitDirect3D::singleton = nullptr;


// ==============================================================
//		windows message handlers
// ==============================================================

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Forward hwnd on because we can get messages (e.g., WM_CREATE)
	// before CreateWindow returns, and thus before mhMainWnd is valid.
	return InitDirect3D::singleton ->MsgProc(hwnd, msg, wParam, lParam);
}
LRESULT InitDirect3D::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			gTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			gTimer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		mViewportWidth = LOWORD(lParam);
		mViewportHeight = HIWORD(lParam);
		if (md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				OnResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					OnResize();
				}

				// Restoring from maximized state?
				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					OnResize();
				}
				else if (mResizing)
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
		mAppPaused = true;
		mResizing = true;
		gTimer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		gTimer.Start();
		OnResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		parentEngine->StopEngine();
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

InitDirect3D::InitDirect3D(HINSTANCE hInstance, StrangeEngine* strangeEngine_Instance)
{
	if (singleton != nullptr)
	{
		std::cout << "[WARN] multiple instances of InitDirect3D were created, releasing previous instance to avoid memory leaks" << std::endl;
		delete singleton;
		singleton = nullptr;

		gLastError = "[WARN] multiple instances of InitDirect3D were created, releasing previous instance to avoid memory leaks";
	}
	if (parentEngine != nullptr)
	{
		std::cout << "[WARN] multiple instances of StrangeEngine were created, releasing previous instance to avoid memory leaks" << std::endl;
		delete parentEngine;
		parentEngine = nullptr;

		gLastError = "[WARN] multiple instances of StrangeEngine were created, releasing previous instance to avoid memory leaks";
	}


	mAppHInstance = hInstance;
	mMainWndCaption = L"StrangeEngine";

	md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;
	mViewportWidth = 800;
	mViewportHeight = 600;
	mEnable4xMsaa = false;
	mHMainWindow = 0;
	mAppPaused = false;
	mMinimized = false;
	mMaximized = false;
	mResizing = false;
	m4xMsaaQuality = 0;

	md3dDevice = 0;
	md3dImmediateContext = 0;
	mSwapChain = 0;
	mDepthStencilBuffer = 0;
	mRenderTargetView = 0;
	mDepthStencilView = 0;

	singleton = this;
	parentEngine = strangeEngine_Instance;
	
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "InitDirect3D instance created: Defaults Set" << std::endl;
	#endif
}

InitDirect3D::~InitDirect3D()
{
	mRenderTargetView->Release();	mRenderTargetView = nullptr;
	mDepthStencilView->Release();	mDepthStencilView = nullptr;
	mSwapChain->Release();			mSwapChain = nullptr;
	mDepthStencilBuffer->Release(); mDepthStencilBuffer = nullptr;

	// Restore all default settings.
	if (md3dImmediateContext)
		md3dImmediateContext->ClearState();

	md3dImmediateContext->Release(); md3dImmediateContext = nullptr;
	md3dDevice->Release(); md3dDevice = nullptr;

	
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "InitDirect3D instance deleted" << std::endl;
	#endif
}

bool InitDirect3D::InitMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mAppHInstance;
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
	RECT R = { 0, 0, mViewportWidth, mViewportHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mHMainWindow = CreateWindow(L"D3DWndClassName", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mAppHInstance, 0);
	if (!mHMainWindow)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(mHMainWindow, SW_SHOW);
	UpdateWindow(mHMainWindow);

	return true;
}

// create the d3dDevice and d3dContext
// return true on success
bool InitDirect3D::CreateDeviceAndContext()
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
		&md3dDevice,							// return the d3dDevice
		&featureLevel,							// return the feature level
		&md3dImmediateContext					// return the d3dDeviceContext
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

bool InitDirect3D::Check4xMSAAQualitySupport()
{
	HRESULT hr = md3dDevice->CheckMultisampleQualityLevels(
		DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);

	// if the above function was unsuccessful
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[WARN]: 4x MSAA Unsupported, disabling 4xMSAA" << std::endl;
		#endif

		mEnable4xMsaa = false;
		return false;
	}

	if (m4xMsaaQuality <= 0)
	{// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[WARN]: 4x MSAA cannot  be negative, disabling 4xMSAA" << std::endl;
		#endif

		mEnable4xMsaa = false;
		return false;
	}

	
	// Debug logs
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "4x MSAA supported, enabling 4xMSAA" << std::endl;
	#endif
	mEnable4xMsaa = true;
	return true;
}

bool InitDirect3D::DescribeSwapChain()
{
	// ==============================================================
	//		describe the swap chain
	// ==============================================================
	
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.OutputWindow = mHMainWindow;                           // set the output windoe of the swap chain to be the window we have already created
    swapChainDesc.Windowed = TRUE;                                // specify TRUE to run inwindowed mode and FALSE to run in fullscreen mode
    swapChainDesc.BufferCount = 1;                                // the number of back buffers we want to use, typically 1 is used fo rback buffering but more can be used
    swapChainDesc.BufferDesc.Width  = mViewportWidth;             // Target window size
    swapChainDesc.BufferDesc.Height = mViewportHeight;            // --"--
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Pixel format of target window
    swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;        // Refresh rate of monitor (provided as fraction = 60/1 here)
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;         // --"--
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // specify how we are going be use the back buffer. because we will be rendering to this back biffer, we specify 'DXGI_USAGE_RENDER_TARGET_OUTPUT'
	// use 4X MSAA
	if (mEnable4xMsaa)
	{
		swapChainDesc.SampleDesc.Count = 4;
		swapChainDesc.SampleDesc.Quality = m4xMsaaQuality-1;
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
	hr = md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**) &dxgiDevice);
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
		md3dDevice,
		&swapChainDesc,
		&mSwapChain
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

void InitDirect3D::CreateRenderTargetView()
{
	// get the back buffer from the swap chain
	ID3D11Texture2D* backBuffer;
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**> (&backBuffer));
	// create the render target view
	md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView);
	// release the back buffer now we are done with it
	backBuffer->Release(); backBuffer = nullptr;

	
	// Debug logs
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "Render target view created " << std::endl;
	#endif
}

bool InitDirect3D::CreateDepthBuffer()
{
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = mViewportWidth;
	depthStencilDesc.Height = mViewportHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	if (mEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
	}
	else
	{
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
	}
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// gDepthStencilBuffer
	// gDepthStencilView

	HRESULT hr;

	hr = md3dDevice->CreateTexture2D(
		&depthStencilDesc,
		0,
		&mDepthStencilBuffer
	);
	// check for failure
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: Could not create septh stencil buffer" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "Could not create septh stencil buffer";
		return false;
	}

	hr = md3dDevice->CreateDepthStencilView(
		mDepthStencilBuffer,
		0,
		&mDepthStencilView
	);
	// check for failure
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: Could not create septh stencil view" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "Could not create septh stencil view";
		return false;
	}

	// Debug logs
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "depth buffer created" << std::endl;
	#endif
	return true;
}

void InitDirect3D::BindViewsToOutputMergerStage()
{
	md3dImmediateContext->OMSetRenderTargets(
		1, &mRenderTargetView, mDepthStencilView
	);

	// Debug logs
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "views bound to output merger stage" << std::endl;
	#endif
}

void InitDirect3D::SetViewport()
{
	D3D11_VIEWPORT vp;

	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(mViewportWidth);
	vp.Height = static_cast<float>(mViewportHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	md3dImmediateContext->RSSetViewports(1, &vp);

	// Debug logs
	#if defined(DEBUG)||defined(_DEBUG)
	std::cout << "viewport set" << std::endl;
	#endif
}


// ==============================================================
//	 Run-time functions
// ==============================================================

int InitDirect3D::Run()
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

			if (!mAppPaused)
			{
				CalculateFrameStats();
				//UpdateScene(mTimer.DeltaTime());
				DrawScene();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}

void InitDirect3D::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.

	static int frameCnt = 0;
	static float timeElapsed = 0.0f;

	frameCnt++;

	// Compute averages over one second period.
	if ((gTimer.GameTime() - timeElapsed) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << mMainWndCaption << L"    "
			<< L"FPS: " << fps << L"    "
			<< L"Frame Time: " << mspf << L" (ms)";
		SetWindowText(mHMainWindow, outs.str().c_str());

		// Reset for next average.
		frameCnt = 0;
		timeElapsed += 1.0f;
	}
}

void InitDirect3D::DrawScene()
{
	if (!(md3dImmediateContext != nullptr && mSwapChain != nullptr))
		return;


	XMVECTORF32 Blue = { 0.0f, 0.0f, 1.0f, 1.0f };
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Blue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	HRESULT hr;
	hr = mSwapChain->Present(0, 0);

	
	// check for failure
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: Could not draw scene" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "Could not draw scene";
		return;
	}

}

void InitDirect3D::OnResize()
{
	if (!(md3dImmediateContext != nullptr &&
		md3dDevice != nullptr &&
		mSwapChain != nullptr))
		return;

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.

	mRenderTargetView->Release(); mRenderTargetView = nullptr;
	mDepthStencilView->Release(); mRenderTargetView = nullptr;
	mDepthStencilBuffer->Release(); mDepthStencilBuffer = nullptr;


	// Resize the swap chain and recreate the render target view.
	HRESULT hr;
	hr = mSwapChain->ResizeBuffers(1, mViewportWidth, mViewportHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
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
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: error accessing depth buffer after resizing" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "error accessing depth buffer after resizing";
	}

	hr = md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView);
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

	depthStencilDesc.Width = mViewportWidth;
	depthStencilDesc.Height = mViewportHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// Use 4X MSAA? --must match swap chain MSAA values.
	if (mEnable4xMsaa)
	{
		depthStencilDesc.SampleDesc.Count = 4;
		depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality - 1;
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

	hr = md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer);
	if (FAILED(hr))
	{
		// Debug logs
		#if defined(DEBUG)||defined(_DEBUG)
		std::cout << "[ERROR]: error creating new depth buffer after resizing" << std::endl;
		#endif

		// return an error message to the engine
		gLastError = "error creating new depth buffer after resizing";
	}

	hr = md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView);
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

	md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);


	// Set the viewport transform.

	mScreenViewport.TopLeftX = 0;
	mScreenViewport.TopLeftY = 0;
	mScreenViewport.Width = static_cast<float>(mViewportWidth);
	mScreenViewport.Height = static_cast<float>(mViewportHeight);
	mScreenViewport.MinDepth = 0.0f;
	mScreenViewport.MaxDepth = 1.0f;
	
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
}
void InitDirect3D::OnMouseDown(WPARAM btnState, int x, int y)
{
}
void InitDirect3D::OnMouseUp(WPARAM btnState, int x, int y)
{
}
void InitDirect3D::OnMouseMove(WPARAM btnState, int x, int y)
{
}
