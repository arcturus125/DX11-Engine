//--------------------------------------------------------------------------------------
// Initialisation of Direct3D and main resources (textures, shaders etc.)
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "Direct3DSetup.h"
#include "Shader.h"
#include "Common.h"
#include <d3d11.h>
#include <vector>


//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
// Globals used to keep code simpler, but try to architect your own code in a better way

// The main Direct3D (D3D) variables
ID3D11Device*        gD3DDevice  = nullptr; // D3D device for overall features
ID3D11DeviceContext* gD3DContext = nullptr; // D3D context for specific rendering tasks

// Swap chain and back buffer
IDXGISwapChain*         gSwapChain              = nullptr;
ID3D11RenderTargetView* gBackBufferRenderTarget = nullptr;

// Depth buffer (can also contain "stencil" values, which we will see later)
ID3D11Texture2D*        gDepthStencilTexture = nullptr; // The texture holding the depth values
ID3D11DepthStencilView* gDepthStencil        = nullptr; // The depth buffer referencing above texture



//--------------------------------------------------------------------------------------
// Initialise / uninitialise Direct3D
//--------------------------------------------------------------------------------------
// Returns false on failure
bool InitDirect3D()
{
    // Many DirectX functions return a "HRESULT" variable to indicate success or failure. Microsoft code often uses
    // the FAILED macro to test this variable, you'll see it throughout the code - it's fairly self explanatory.
    HRESULT hr = S_OK;


    //// Initialise DirectX ////

    /* this snippet of code creates a description for the swap chain
     * this description describes the bahaviour of the back buffer
     */

        // create a swap-chain (create a back buffer to render to)
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.OutputWindow = gHWnd;                           // set the output windoe of the swap chain to be the window we have already created
        swapChainDesc.Windowed = TRUE;                                // specify TRUE to run inwindowed mode and FALSE to run in fullscreen mode
        swapChainDesc.BufferCount = 1;                                // the number of back buffers we want to use, typically 1 is used fo rback buffering but more can be used
        swapChainDesc.BufferDesc.Width  = gViewportWidth;             // Target window size
        swapChainDesc.BufferDesc.Height = gViewportHeight;            // --"--
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // Pixel format of target window
        swapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;        // Refresh rate of monitor (provided as fraction = 60/1 here)
        swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;         // --"--
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // specify how we are going be use the back buffer. because we will be rendering to this back biffer, we specify 'DXGI_USAGE_RENDER_TARGET_OUTPUT'
        swapChainDesc.SampleDesc.Count   = 1;
        swapChainDesc.SampleDesc.Quality = 0;


        UINT flags = 0; // 

        // runs if and ONLY if in DEBUG mode
        #if defined(DEBUG)||defined(_DEBUG)
                // if in debug mode, Set the flags to D3D11_CREATE_DEVICE_DEBUG 
                // to get more debugging information (in the "Output" window of Visual Studio)
                flags = D3D11_CREATE_DEVICE_DEBUG; 
        #endif

    /* this snippet of code creates  the ID3D11Device and ID3D11DeviceContext
     */
        // Create a Direct3D device(i.e.initialise D3D)
        hr = D3D11CreateDeviceAndSwapChain(
            nullptr, // specity null because we want to use the primary display adapter
            D3D_DRIVER_TYPE_HARDWARE, // specify 'D3D_DRIVER_TYPE_HARDWARE' to use hardware acelleration
            0, // speficy null because we are using the hardware for rendering
            flags, // use flags above
            0,  // specify null to use the greated supported feature level
            0,  //
            D3D11_SDK_VERSION, // use the DX11 SDK
            &swapChainDesc, 
            &gSwapChain, 
            &gD3DDevice, // returns the created ID3D11Device
            nullptr, 
            &gD3DContext); // returns the created ID3D11DeviceContext

        // if there was an error with the above function, set appropriate error message
        if (FAILED(hr))
        {
            gLastError = "Error creating Direct3D device";
            return false;
        }


    /* in order to bind the back buffer to the output merger stage of the pipeline 
     * (So Direct3D can render to into it),
     * we need to create a render target view to the back buffer
     */
        ID3D11Texture2D* backBuffer;                                                        // the back buffer is obtained by accessing the swap chain's GetBuffer method, 
        hr = gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);     // the first parameter is an index, we only use 1 back buffer so we use index 0
        if (FAILED(hr)){ gLastError = "Error creating swap chain"; return false; }          // return error message if there is a problem
        hr = gD3DDevice->CreateRenderTargetView(backBuffer, NULL, &gBackBufferRenderTarget);// create the render target view for the back buffer
        backBuffer->Release();                                                              // delete the back buffer variable now we are done with it
        if (FAILED(hr)){ gLastError = "Error creating render target view"; return false; }  // return error message if there is a problem


    //// Create depth buffer to go along with the back buffer ////
    
    // First create a texture to hold the depth buffer values
    D3D11_TEXTURE2D_DESC dbDesc = {};
    dbDesc.Width  = gViewportWidth; // depth buffer's texture is the Same size as viewport / back-buffer
    dbDesc.Height = gViewportHeight;// --"--
    dbDesc.MipLevels = 1;
    dbDesc.ArraySize = 1; // the depth buffer is just 1 image so array length is 1
    dbDesc.Format = DXGI_FORMAT_D32_FLOAT; // Each depth (pixel) value is a single float
    dbDesc.SampleDesc.Count = 1;
    dbDesc.SampleDesc.Quality = 0;
    dbDesc.Usage = D3D11_USAGE_DEFAULT;
    dbDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; // specify this texture is for use as a depth buffer
    dbDesc.CPUAccessFlags = 0;
    dbDesc.MiscFlags = 0;
    hr = gD3DDevice->CreateTexture2D(&dbDesc, nullptr, &gDepthStencilTexture);
    if (FAILED(hr))
    {
        gLastError = "Error creating depth buffer texture";
        return false;
    }

    // Create the depth stencil view - an object to allow us to use the texture
    // just created as a depth buffer
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = dbDesc.Format;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    hr = gD3DDevice->CreateDepthStencilView(gDepthStencilTexture, &dsvDesc,
                                            &gDepthStencil);
    if (FAILED(hr))
    {
        gLastError = "Error creating depth buffer view";
        return false;
    }
    
    return true;
}


// Release the memory held by all objects created
void ShutdownDirect3D()
{
    // Release each Direct3D object to return resources to the system. Missing these out will cause memory
    // leaks. Check documentation to see which objects need to be released when adding new features in your
    // own projects.
    if (gD3DContext)
    {
        gD3DContext->ClearState(); // This line is also needed to reset the GPU before shutting down DirectX
        gD3DContext->Release();
    }
    if (gDepthStencil)           gDepthStencil->Release();
    if (gDepthStencilTexture)    gDepthStencilTexture->Release();
    if (gBackBufferRenderTarget) gBackBufferRenderTarget->Release();
    if (gSwapChain)              gSwapChain->Release();
    if (gD3DDevice)              gD3DDevice->Release();
}


