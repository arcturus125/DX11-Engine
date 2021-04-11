//--------------------------------------------------------------------------------------
// Scene geometry and layout preparation
// Scene rendering & update
//--------------------------------------------------------------------------------------

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
//#include "Light.h"


//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------
// Addition of Mesh, Model and Camera classes have greatly simplified this section
// Geometry data has gone to Mesh class. Positions, rotations, matrices have gone to Model and Camera classes

// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 2.0f;  // 2 radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // 50 units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)


// Meshes, models and cameras, same meaning as TL-Engine. Meshes prepared in InitGeometry function, Models & camera in InitScene
Mesh* gTeapotMesh;
Mesh* gSphereMesh;
Mesh* gCubeMesh;
Mesh* gBumpedCubeMesh;
Mesh* gCrateMesh;
Mesh* gGroundMesh;
Mesh* gLightMesh;

Model* gTeapot;
Model* gSphere;
Model* gCube;
Model* gTest;
Model* gBumpedCube;
Model* gCrate;
Model* gGround;

Camera* gCamera;


const int NUM_LIGHTS = 2;
Light* gLights[NUM_LIGHTS]; 


// Additional light information
CVector3 gAmbientColour = { 0.2f, 0.2f, 0.3f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
float    gSpecularPower = 256; // Specular power controls shininess - same for all models in this app

ColourRGBA gBackgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f };

// Variables controlling light1's orbiting of the cube
const float gLightOrbit = 20.0f;
const float gLightOrbitSpeed = 0.7f;

// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
bool lockFPS = true;


//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
// Variables sent over to the GPU each frame
// The structures are now in Common.h
// IMPORTANT: Any new data you add in C++ code (CPU-side) is not automatically available to the GPU
//            Anything the shaders need (per-frame or per-model) needs to be sent via a constant buffer

PerFrameConstants gPerFrameConstants;      // The constants that need to be sent to the GPU each frame (see common.h for structure)
ID3D11Buffer*     gPerFrameConstantBuffer; // The GPU buffer that will recieve the constants above

PerModelConstants gPerModelConstants;      // As above, but constant that change per-model (e.g. world matrix)
ID3D11Buffer*     gPerModelConstantBuffer; // --"--



//--------------------------------------------------------------------------------------
// Textures
//--------------------------------------------------------------------------------------

// DirectX objects controlling textures used in this lab
ID3D11Resource*           gCharacterDiffuseSpecularMap    = nullptr; // This object represents the memory used by the texture on the GPU
ID3D11ShaderResourceView* gCharacterDiffuseSpecularMapSRV = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

ID3D11Resource*           gWoodDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gWoodDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gCrateDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gCrateDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gGroundDiffuseSpecularMap    = nullptr;
ID3D11ShaderResourceView* gGroundDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gLightDiffuseMap    = nullptr;
ID3D11ShaderResourceView* gLightDiffuseMapSRV = nullptr;

ID3D11Resource*           gPatternDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gPatternDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gPatternNormalMap = nullptr;
ID3D11ShaderResourceView* gPatternNormalMapSRV = nullptr;

ID3D11Resource*           gCobbleDiffuseSpecularMap = nullptr;
ID3D11ShaderResourceView* gCobbleDiffuseSpecularMapSRV = nullptr;

ID3D11Resource*           gCobbleNormalHeightMap = nullptr;
ID3D11ShaderResourceView* gCobbleNormalHeightMapSRV = nullptr;


float gParallaxDepth = 0.08f; // Overall depth of bumpiness for parallax mapping
bool gUseParallax = true;  // Toggle for parallax 

//--------------------------------------------------------------------------------------
// Initialise scene geometry, constant buffers and states
//--------------------------------------------------------------------------------------

// Prepare the geometry required for the scene
// Returns true on success
bool InitGeometry()
{
    // Load mesh geometry data, just like TL-Engine this doesn't create anything in the scene. Create a Model for that.
    // IMPORTANT NOTE: Will only keep the first object from the mesh - multipart objects will have parts missing - see later lab for more robust loader
    try 
    {
        gTeapotMesh = new Mesh("teapot.x");
        gSphereMesh = new Mesh("Sphere.x");
        gCubeMesh = new Mesh("Cube.x");
        gBumpedCubeMesh = new Mesh("Cube.x", true);     // <-----   use true to make this mesh generate tangents
                                                        //          this means that the model will use TangentVertex in common.hlsli instead of
                                                        //          BasicVertex. (meaning that normal maps can now be used on the any model using this mesh)
        gCrateMesh    = new Mesh("CargoContainer.x");
        gGroundMesh   = new Mesh("Hills.x", true);
        gLightMesh    = new Mesh("Light.x");
    }
    catch (std::runtime_error e)  // Constructors cannot return error messages so use exceptions to catch mesh errors (fairly standard approach this)
    {
        gLastError = e.what(); // This picks up the error message put in the exception (see Mesh.cpp)
        return false;
    }


    // Load the shaders required for the geometry we will use (see Shader.cpp / .h)
    if (!LoadShaders())
    {
        gLastError = "Error loading shaders";
        return false;
    }


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


    //// Load / prepare textures on the GPU ////

    // Load textures and create DirectX objects for them
    // The LoadTexture function requires you to pass a ID3D11Resource* (e.g. &gCubeDiffuseMap), which manages the GPU memory for the
    // texture and also a ID3D11ShaderResourceView* (e.g. &gCubeDiffuseMapSRV), which allows us to use the texture in shaders
    // The function will fill in these pointers with usable data. The variables used here are globals found near the top of the file.
    if (!LoadTexture("StoneDiffuseSpecular.dds",  &gCharacterDiffuseSpecularMap, &gCharacterDiffuseSpecularMapSRV) ||
        !LoadTexture("PatternDiffuseSpecular.dds",&gPatternDiffuseSpecularMap, &gPatternDiffuseSpecularMapSRV) ||
        !LoadTexture("PatternNormal.dds",         &gPatternNormalMap, &gPatternNormalMapSRV) ||
        !LoadTexture("CobbleDiffuseSpecular.dds", &gCobbleDiffuseSpecularMap, &gCobbleDiffuseSpecularMapSRV) ||
        !LoadTexture("CobbleNormalHeight.dds",    &gCobbleNormalHeightMap, &gCobbleNormalHeightMapSRV) ||
        !LoadTexture("WoodDiffuseSpecular.dds", &gWoodDiffuseSpecularMap, &gWoodDiffuseSpecularMapSRV) ||
        !LoadTexture("CargoA.dds",               &gCrateDiffuseSpecularMap,     &gCrateDiffuseSpecularMapSRV    ) ||
        !LoadTexture("GrassDiffuseSpecular.dds", &gGroundDiffuseSpecularMap,    &gGroundDiffuseSpecularMapSRV   ) ||
        !LoadTexture("Flare.jpg",                &gLightDiffuseMap,             &gLightDiffuseMapSRV))
    {
        gLastError = "Error loading textures";
        return false;
    }


  	// Create all filtering modes, blending modes etc. used by the app (see State.cpp/.h)
	if (!CreateStates())
	{
		gLastError = "Error creating states";
		return false;
	}

	return true;
}


// Prepare the scene
// Returns true on success
bool InitScene()
{
    //// Set up scene ////

    gTeapot   = new Model(gTeapotMesh);
    gSphere     = new Model(gSphereMesh);
    gCube = new Model(gCubeMesh);
    gBumpedCube = new Model(gBumpedCubeMesh);
    gCrate    = new Model(gCrateMesh);
    gGround   = new Model(gGroundMesh);
    gTest = new Model(gCubeMesh);


	// Initial positions
	gTeapot->SetPosition({ 20, 0, 0 });
    gTeapot->SetScale(1); 
    gTeapot->SetRotation({ 0, ToRadians(135.0f), 0 });
    gSphere->SetPosition({ 10,10,10 });
    gTest->SetPosition({ 10,20,10 });
    gCube->SetPosition({ 30,20,10 });
    gBumpedCube->SetPosition({ 60,30,20 });
	gCrate-> SetPosition({ 45, 0, 45 });
	gCrate-> SetScale(6);
	gCrate-> SetRotation({ 0.0f, ToRadians(-50.0f), 0.0f });


    // Light set-up - using an array this time
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gLights[i] = new Light();
        gLights[i]->model = new Model(gLightMesh);
    }

    gLights[0]->type = Light::LightType::Spot;
    gLights[0]->colour = { 0.8f, 0.8f, 1.0f };
    gLights[0]->strength = 10;
    gLights[0]->model->SetPosition({ 30, 20, 0 });
    gLights[0]->model->SetScale(pow(gLights[0]->strength, 0.7f)); // Convert light strength into a nice value for the scale of the light - equation is ad-hoc.


    gLights[1]->type = Light::LightType::Point;
    gLights[1]->colour = { 1.0f, 0.8f, 0.2f };
    gLights[1]->strength = 40;
    gLights[1]->model->SetPosition({ -20, 50, 20 });
    gLights[1]->model->SetScale(pow(gLights[1]->strength, 0.7f));


    //// Set up camera ////

    gCamera = new Camera();
    gCamera->SetPosition({ 15, 30,-70 });
    gCamera->SetRotation({ ToRadians(13), 0, 0 });

    return true;
}


// Release the geometry and scene resources created above
void ReleaseResources()
{
    ReleaseStates();

    if (gLightDiffuseMapSRV)             gLightDiffuseMapSRV->Release();
    if (gLightDiffuseMap)                gLightDiffuseMap->Release();
    if (gGroundDiffuseSpecularMapSRV)    gGroundDiffuseSpecularMapSRV->Release();
    if (gGroundDiffuseSpecularMap)       gGroundDiffuseSpecularMap->Release();
    if (gCrateDiffuseSpecularMapSRV)     gCrateDiffuseSpecularMapSRV->Release();
    if (gCrateDiffuseSpecularMap)        gCrateDiffuseSpecularMap->Release();
    if (gCharacterDiffuseSpecularMapSRV) gCharacterDiffuseSpecularMapSRV->Release();
    if (gCharacterDiffuseSpecularMap)    gCharacterDiffuseSpecularMap->Release();
    if (gPatternDiffuseSpecularMapSRV) gPatternDiffuseSpecularMapSRV->Release();
    if (gPatternDiffuseSpecularMap) gPatternDiffuseSpecularMap->Release();
    if (gPatternNormalMapSRV) gPatternNormalMapSRV->Release();
    if (gPatternNormalMap) gPatternNormalMap->Release();
    if (gCobbleDiffuseSpecularMap) gCobbleDiffuseSpecularMap->Release();
    if (gCobbleDiffuseSpecularMapSRV) gCobbleDiffuseSpecularMapSRV->Release();
    if (gCobbleNormalHeightMap) gCobbleNormalHeightMap->Release();
    if (gCobbleNormalHeightMapSRV) gCobbleNormalHeightMapSRV->Release();

    if (gWoodDiffuseSpecularMap) gWoodDiffuseSpecularMap->Release();
    if (gWoodDiffuseSpecularMapSRV) gWoodDiffuseSpecularMapSRV->Release();

    if (gPerModelConstantBuffer)  gPerModelConstantBuffer->Release();
    if (gPerFrameConstantBuffer)  gPerFrameConstantBuffer->Release();

    ReleaseShaders();

    // See note in InitGeometry about why we're not using unique_ptr and having to manually delete
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        delete gLights[i]->model;  gLights[i]->model = nullptr;
        delete gLights[i];
    }
    delete gCamera;    gCamera    = nullptr;
    delete gGround;    gGround    = nullptr;
    delete gCrate;     gCrate     = nullptr;
    delete gTeapot;  gTeapot = nullptr;
    delete gSphere; gSphere = nullptr;
    delete gCube; gCube = nullptr;

    delete gLightMesh;     gLightMesh     = nullptr;
    delete gGroundMesh;    gGroundMesh    = nullptr;
    delete gCrateMesh;     gCrateMesh     = nullptr;
    delete gTeapotMesh;  gTeapotMesh = nullptr;
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


    //// Render lit models ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gPixelLightingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gPixelLightingPixelShader,  nullptr, 0);
    
    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullBackState);

    // Select the approriate textures and sampler to use in the pixel shader
    gD3DContext->PSSetShaderResources(0, 1, &gGroundDiffuseSpecularMapSRV); // First parameter must match texture slot number in the shader
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);


    gD3DContext->PSSetShaderResources(0, 1, &gWoodDiffuseSpecularMapSRV);// pass second texture to shader
    gTest->Render();


    // Render other lit models, only change textures for each onee
    gD3DContext->PSSetShaderResources(0, 1, &gCharacterDiffuseSpecularMapSRV); 
    gTeapot->Render();

    gD3DContext->PSSetShaderResources(0, 1, &gCrateDiffuseSpecularMapSRV);
    gCrate->Render();





    //sphere
    gD3DContext->VSSetShader(gWiggleVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gWigglePixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &gCharacterDiffuseSpecularMapSRV);
    gSphere->Render();

    //cube
    gD3DContext->VSSetShader(gFadingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gFadingPixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &gCharacterDiffuseSpecularMapSRV);// pass first texture to shader
    gD3DContext->PSSetShaderResources(1, 1, &gWoodDiffuseSpecularMapSRV);// pass second texture to shader
    gCube->Render();

    //bumped cube
    gD3DContext->VSSetShader(gNormalMappingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gNormalMappingPixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &gPatternDiffuseSpecularMapSRV);// pass first texture to shader
    gD3DContext->PSSetShaderResources(1, 1, &gPatternNormalMapSRV);// pass second texture to shader
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);
    gBumpedCube->Render();


    // paralax mapped ground
    gD3DContext->VSSetShader(gParallaxMappingVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gParallaxMappingPixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &gCobbleDiffuseSpecularMapSRV);// pass first texture to shader
    gD3DContext->PSSetShaderResources(1, 1, &gCobbleNormalHeightMapSRV);// pass second texture to shader
    gGround->Render();




    //// Render lights ////

    // Select which shaders to use next
    gD3DContext->VSSetShader(gBasicTransformVertexShader, nullptr, 0);
    gD3DContext->PSSetShader(gLightModelPixelShader,      nullptr, 0);

    // Select the texture and sampler to use in the pixel shader
    gD3DContext->PSSetShaderResources(0, 1, &gLightDiffuseMapSRV); // First parameter must match texture slot number in the shaer
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);

    // States - additive blending, read-only depth buffer and no culling (standard set-up for blending
    gD3DContext->OMSetBlendState(gAdditiveBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gDepthReadOnlyState, 0);
    gD3DContext->RSSetState(gCullNoneState);

    // Render all the lights in the array
    for (int i = 0; i < NUM_LIGHTS; ++i)
    {
        gPerModelConstants.objectColour = gLights[i]->colour; // Set any per-model constants apart from the world matrix just before calling render (light colour here)
        gLights[i]->model->Render();
    }
}


float wiggleTimer = 0;

// Rendering the scene
void RenderScene()
{
    //// Common settings ////

    // Set up the light information in the constant buffer
    // Don't send to the GPU yet, the function RenderSceneFromCamera will do that

    int numOfPointLights = 0;
    int numOfSpotLights = 0;
    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        if (gLights[i]->type == Light::LightType::Point) 
        {
            gPerFrameConstants.light[numOfPointLights] = gLights[i]->GetPointLightData();
            numOfPointLights++; 
        }
        if (gLights[i]->type == Light::LightType::Spot)
        {
            gPerFrameConstants.spotLight[numOfSpotLights] = gLights[i]->GetSpotLightData();
            numOfSpotLights++;
        }
    }
    /*for (int i = 0; i < NUM_LIGHTS; i++)
    {
        gPerFrameConstants.light[i] = gLights[i]->GetPointLightData();
    }*/

    gPerFrameConstants.ambientColour  = gAmbientColour;
    gPerFrameConstants.specularPower  = gSpecularPower;
    gPerFrameConstants.cameraPosition = gCamera->Position();
    gPerFrameConstants.parallaxDepth = (gUseParallax ? gParallaxDepth : 0);
    gPerFrameConstants.timer = wiggleTimer;
    gPerFrameConstants.numLights = numOfPointLights;
    gPerFrameConstants.numSpotLights = numOfSpotLights;



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
    RenderSceneFromCamera(gCamera);


    //// Scene completion ////

    // When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
    // Set first parameter to 1 to lock to vsync (typically 60fps)
    gSwapChain->Present(lockFPS ? 1 : 0, 0);
}


//--------------------------------------------------------------------------------------
// Scene Update
//--------------------------------------------------------------------------------------


int strengthMultiplier = 1;
int colourMultiplier = 1;

// Update models and camera. frameTime is the time passed since the last frame
void UpdateScene(float frameTime)
{
    wiggleTimer += frameTime;               // a timer used for the wiggle in the pixel shader 40%
    if (wiggleTimer > 1000000) wiggleTimer = 0; //


	// Control sphere (will update its world matrix)
    gTeapot->Control(0, frameTime, Key_I, Key_K, Key_J, Key_L, Key_U, Key_O, Key_Period, Key_Comma);

    // Orbit the light - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
	static float rotate = 0.0f;
    static bool go = true;
	gLights[0]->model->SetPosition( gTeapot->Position() + CVector3{ cos(rotate) * gLightOrbit, 10, sin(rotate) * gLightOrbit } );
    if (go)  rotate -= gLightOrbitSpeed * frameTime;
    if (KeyHit(Key_1))  go = !go;

	// Control camera (will update its view matrix)
	gCamera->Control(frameTime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D );


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

    // MYCODE
    gLights[1]->strength -= 0.2f * strengthMultiplier; // 40%
    if (gLights[1]->strength <= 0)                    // make one light pulsate on and off
        strengthMultiplier = -1;                      //
    else if (gLights[1]->strength >= 200)             //
        strengthMultiplier = 1;                       //



    gLights[0]->colour.x -= 0.003f  * colourMultiplier; // 40%
    if (gLights[0]->colour.x <= 0)                    // make the other light gradually change colour between blue and white
        colourMultiplier = -1;                        //
    else if (gLights[0]->colour.x >= 1)               //
        colourMultiplier = 1;                         //

}
