//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------


#include "pch.h"
#include "Scene.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "State.h"
#include "Shader.h"
#include "Input.h"
#include "Common.h"

#include "CVector2.h" 
#include "CVector3.h" 
#include "CMatrix4x4.h"
#include "MathHelpers.h"     // Helper functions for maths
#include "GraphicsHelpers.h" // Helper functions to unclutter the code here

#include "ColourRGBA.h" 

#include <sstream>
#include <memory>
#include "Texture.h"



Camera* mainCamera;

// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
bool lockFPS = true;

// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 2.0f;  // 2 radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // 50 units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)


float wiggleTimer = 0;

//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------
std::vector <Model*> autoRenderList;

Mesh* gLightMesh;




//--------------------------------------------------------------------------------------
// Lighting Data
//--------------------------------------------------------------------------------------
std::vector <Light*> gLights;


// lighting settings
CVector3 gAmbientColour = { 0.2f, 0.2f, 0.3f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
float    gSpecularPower = 256; // Specular power controls shininess - same for all models in this app

ColourRGBA gBackgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f };




//--------------------------------------------------------------------------------------
// Constant Buffers: Variables sent over to the GPU each frame
//--------------------------------------------------------------------------------------

PerFrameConstants gPerFrameConstants;      // The constants that need to be sent to the GPU each frame (see common.h for structure)
ID3D11Buffer*     gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constant that change per-model (e.g. world matrix)
ID3D11Buffer*     gPerModelConstantBuffer; // --"--



//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------
std::vector <Texture*> textures;

Texture* lightTexture       = nullptr;



//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------
std::vector <Shader*> shaders;

Shader* BasicTransformShader    = nullptr; // shaders used for lights
Shader* LightModelShader        = nullptr; //


float gParallaxDepth = 0.08f; // Overall depth of bumpiness for parallax mapping
bool gUseParallax = true;  // Toggle for parallax 


// Cell shading data
CVector3 OutlineColour = { 0, 0, 0 };
float    OutlineThickness = 0.015f;

//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry()
{
    BasicTransformShader    = new Shader("BasicTransform", false, true);
    LightModelShader        = new Shader("LightModel", true, false);

    lightTexture = new Texture("Flare.jpg");



    // Create GPU-side constant buffers to receive the gPerFrameConstants and gPerModelConstants structures above
    // These allow us to pass data from CPU to shaders such as lighting information or matrices
    // See the comments above where these variable are declared and also the UpdateScene function
    gPerFrameConstantBuffer = CreateConstantBuffer(sizeof(gPerFrameConstants));
    gPerModelConstantBuffer = CreateConstantBuffer(sizeof(gPerModelConstants));
    if (gPerFrameConstantBuffer == nullptr || gPerModelConstantBuffer == nullptr)
    {
        gLastError = "Error creating constant buffers";
        return false;
    }

  	// Create all filtering modes, blending modes etc.
	if (!CreateStates())
	{
		gLastError = "Error creating states";
		return false;
	}

	return true;
}



// Release the geometry and scene resources created above
void ReleaseResources()
{
    ReleaseStates();

    // loop through textures and release their contents, then clear the list
    for (int i = 0; i < textures.size(); i++)
    {
        textures[i]->gTextureMap->Release();
        textures[i]->gTextureMapSRV->Release();
    }
    textures.clear();

    if (gPerModelConstantBuffer)  gPerModelConstantBuffer->Release();
    if (gPerFrameConstantBuffer)  gPerFrameConstantBuffer->Release();
    // loop through the shader and release their contents, then clear the list
    for (int i = 0; i < shaders.size(); i++)
    {
        shaders[i]->pixelShader->Release();
        shaders[i]->vertexShader->Release();
    }
    shaders.clear();

    // loop through lights and delete their contents
    for (int i = 0; i < gLights.size(); ++i)
    {
        delete gLights[i]->model;  gLights[i]->model = nullptr;
        delete gLights[i];
    }
}



//--------------------------------------------------------------------------------------
// Scene Rendering
//--------------------------------------------------------------------------------------

// Render everything in the scene from the given camera
void RenderSceneFromCamera(Camera* camera)
{
    // Set camera matrices in the constant buffer and send over to GPU
    gPerFrameConstants.viewMatrix           = camera->ViewMatrix();
    gPerFrameConstants.projectionMatrix     = camera->ProjectionMatrix();
    gPerFrameConstants.viewProjectionMatrix = camera->ViewProjectionMatrix();
    UpdateConstantBuffer(gPerFrameConstantBuffer, gPerFrameConstants);

    // Indicate that the constant buffer we just updated is for use in the vertex shader (VS) and pixel shader (PS)
    gD3DContext->VSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer); // First parameter must match constant buffer number in the shader 
    gD3DContext->PSSetConstantBuffers(0, 1, &gPerFrameConstantBuffer);

    for (int i = 0; i < autoRenderList.size(); i++)
    {
        for (int j = 0; j < autoRenderList[i]->renderPass.size(); j++)
        {
            autoRenderList[i]->AutoRender(gD3DContext, j);
        }
    }


    //// Render lights ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(BasicTransformShader->vertexShader, nullptr, 0);
    gD3DContext->PSSetShader(LightModelShader->pixelShader,      nullptr, 0);

    // Select the texture and sampler to use in the pixel shader
    gD3DContext->PSSetShaderResources(0, 1, &lightTexture->gTextureMapSRV); // First parameter must match texture slot number in the shaer
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // States - additive blending, read-only depth buffer and no culling (standard set-up for blending
    gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);

    // Render all the lights in the array
    for (int i = 0; i < gLights.size(); ++i)
    {
        gPerModelConstants.objectColour = gLights[i]->colour; // Set any per-model constants apart from the world matrix just before calling render (light colour here)
        gLights[i]->model->Render();
    }
}



// Rewdndering the scene
bool RenderScene()
{
    if (mainCamera == nullptr)
    {
        gLastError = "No main camera set, please remember to set your camera as the mainCamera with SetMainCamera(myCamera) at the start of the engine";
        return false;
    }
    //// set up gPerFrameConstants ready to be sent to the GPU
    int numOfPointLights = 0;
    int numOfSpotLights = 0;
    int numOfDirectionalLights = 0;
    for (int i = 0; i < gLights.size(); i++)
    {
        if (gLights[i]->type == Light::LightType::Point) 
        {
            gPerFrameConstants.light[numOfPointLights] = gLights[i]->GetPointLightData();
            numOfPointLights++; 
        }
        else if (gLights[i]->type == Light::LightType::Spot)
        {
            gPerFrameConstants.spotLight[numOfSpotLights] = gLights[i]->GetSpotLightData();
            numOfSpotLights++;
        }
        else if (gLights[i]->type == Light::LightType::Directional)
        {

            gPerFrameConstants.directionalLight[numOfDirectionalLights] = gLights[i]->GetDirectionalLightData();
            numOfDirectionalLights++;
        }
    }

    gPerFrameConstants.ambientColour  = gAmbientColour;
    gPerFrameConstants.specularPower  = gSpecularPower;
    gPerFrameConstants.cameraPosition = mainCamera->Position();
    gPerFrameConstants.parallaxDepth = (gUseParallax ? gParallaxDepth : 0);
    gPerFrameConstants.timer = wiggleTimer;
    gPerFrameConstants.outlineColour = OutlineColour;
    gPerFrameConstants.outlineThickness = OutlineThickness;
    gPerFrameConstants.numLights = numOfPointLights;
    gPerFrameConstants.numSpotLights = numOfSpotLights;
    gPerFrameConstants.numDirectionalLights = numOfDirectionalLights;



    //// Main scene rendering ////

    // Set the back buffer as the target for rendering and select the main depth buffer.
    // When finished the back buffer is sent to the "front buffer" - which is the monitor.
    gD3DContext->OMSetRenderTargets(1, &gBackBufferRenderTarget, gDepthStencil);

    // Clear the back buffer to a fixed colour and the depth buffer to the far distance
    gD3DContext->ClearRenderTargetView(gBackBufferRenderTarget, &gBackgroundColor.r);
    gD3DContext->ClearDepthStencilView(gDepthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

    // Setup the viewport to the size of the main window
    D3D11_VIEWPORT vp;
    vp.Width  = static_cast<FLOAT>(gViewportWidth);
    vp.Height = static_cast<FLOAT>(gViewportHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gD3DContext->RSSetViewports(1, &vp);

    // Render the scene from the main camera
    RenderSceneFromCamera(mainCamera);


    //// Scene completion ////

    // When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
    // Set first parameter to 1 to lock to vsync (typically 60fps)
    gSwapChain->Present(lockFPS ? 1 : 0, 0);
    return true;
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------



// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
    wiggleTimer += frameTime;               // a timer used for the wiggle in the pixel shader 40%
    if (wiggleTimer > 1000000) wiggleTimer = 0; //


	// Control sphere (will update its world matrix)
    //gTeapot->Control(0, frameTime, Key_I, Key_K, Key_J, Key_L, Key_U, Key_O, Key_Period, Key_Comma);

    // Toggle FPS limiting
    if (KeyHit(Key_P))  lockFPS = !lockFPS;

    // Show frame time / FPS in the window title //
    const float fpsUpdateTime = 0.5f; // How long between updates (in seconds)
    static float totalFrameTime = 0;
    static int frameCount = 0;
    totalFrameTime += frameTime;
    ++frameCount;
    if (totalFrameTime > fpsUpdateTime)
    {
        // Displays FPS rounded to nearest int, and frame time (more useful for developers) in milliseconds to 2 decimal places
        float avgFrameTime = totalFrameTime / frameCount;
        std::ostringstream frameTimeMs;
        frameTimeMs.precision(2);
        frameTimeMs << std::fixed << avgFrameTime * 1000;
        std::string windowTitle = "StrangeEngine - Frame Time: " + frameTimeMs.str() +
                                  "ms, FPS: " + std::to_string(static_cast<int>(1 / avgFrameTime + 0.5f));
        SetWindowTextA(gHWnd, windowTitle.c_str());
        totalFrameTime = 0;
        frameCount = 0;
    }

}
