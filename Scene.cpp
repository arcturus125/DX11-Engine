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
#include "Texture.h"



// Lock FPS to monitor refresh rate, which will typically set it to 60fps. Press 'p' to toggle to full fps
bool lockFPS = true;

// Constants controlling speed of movement/rotation (measured in units per second because we're using frame time)
const float ROTATION_SPEED = 2.0f;  // 2 radians per second for rotation
const float MOVEMENT_SPEED = 50.0f; // 50 units per second for movement (what a unit of length is depends on 3D model - i.e. an artist decision usually)


float wiggleTimer = 0;

//--------------------------------------------------------------------------------------
// Scene Data
//--------------------------------------------------------------------------------------

Mesh* gTeapotMesh;
Mesh* gSphereMesh;
Mesh* gCubeMesh;
Mesh* gBumpedCubeMesh;
Mesh* gCrateMesh;
Mesh* gGroundMesh;
Mesh* gLightMesh;
Mesh* gTrollMesh;

Model* gTeapot;
Model* gSphere;
Model* gCube;
Model* gTest;
Model* gBumpedCube;
Model* gCrate;
Model* gGround;
Model* gTroll;

Camera* gCamera;


//--------------------------------------------------------------------------------------
// Lighting Data
//--------------------------------------------------------------------------------------
std::vector <Light*> gLights;


// lighting settings
CVector3 gAmbientColour = { 0.2f, 0.2f, 0.3f }; // Background level of light (slightly bluish to match the far background, which is dark blue)
float    gSpecularPower = 256; // Specular power controls shininess - same for all models in this app

ColourRGBA gBackgroundColor = { 0.2f, 0.2f, 0.3f, 1.0f };

// Variables controlling light1's orbiting of the cube
const float gLightOrbit = 20.0f;
const float gLightOrbitSpeed = 0.7f;

int strengthMultiplier = 1;
int colourMultiplier = 1;


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

Texture* characterTexture   = nullptr;
Texture* woodTexture        = nullptr;
Texture* crateTexture       = nullptr;
Texture* grassTexture       = nullptr;
Texture* lightTexture       = nullptr;
Texture* patternTexture     = nullptr;
Texture* paternNormalMap    = nullptr;
Texture* cobbleTexture      = nullptr;
Texture* cobbleNormalMap    = nullptr;
Texture* alphaTexture       = nullptr;
Texture* trolltexture       = nullptr;
Texture* cellMap            = nullptr;



//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------
std::vector <Shader*> shaders;

Shader* wiggleShader            = nullptr;
Shader* fadingShader            = nullptr;
Shader* normalMappingShader     = nullptr;
Shader* parallaxMappingShader   = nullptr;
Shader* defaultShader           = nullptr;
Shader* cellShading             = nullptr;
Shader* cellShadingOutline      = nullptr;

Shader* AlphaBlendingShader     = nullptr;
Shader* BasicTransformShader    = nullptr;
Shader* LightModelShader        = nullptr;


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
    // #############################
    //  load Meshes
    // #############################
    try 
    {
        gTeapotMesh     = new Mesh("teapot.x");
        gSphereMesh     = new Mesh("Sphere.x");
        gCubeMesh       = new Mesh("Cube.x");
        gTrollMesh      = new Mesh("Troll.x");
        gBumpedCubeMesh = new Mesh("Cube.x", true);     // <-----   use true to make this mesh generate tangents
                                                        //          this means that the model will use TangentVertex in common.hlsli instead of
                                                        //          BasicVertex. (meaning that normal maps can now be used on the any model using this mesh)
        gCrateMesh      = new Mesh("CargoContainer.x");
        gGroundMesh     = new Mesh("Hills.x", true);
    }
    // if there is an error loading any of these meshes, display error message to user
    catch (std::runtime_error e)
    {
        gLastError = e.what();
    }


    // #############################
    //  load shaders
    // #############################

    try
    {
        wiggleShader            = new Shader("Wiggle");
        fadingShader            = new Shader("Fading");
        normalMappingShader     = new Shader("NormalMapping");
        parallaxMappingShader   = new Shader("ParallaxMapping");
        defaultShader           = new Shader("PixelLighting");
        cellShading             = new Shader("CellShading");
        cellShadingOutline      = new Shader("CellShadingOutline");

        AlphaBlendingShader     = new Shader("AlphaBlending", true, false);
        BasicTransformShader    = new Shader("BasicTransform", false, true);
        LightModelShader        = new Shader("LightModel", true, false);

        // add each shader to the list of shaders
        shaders.push_back(wiggleShader);
        shaders.push_back(fadingShader);
        shaders.push_back(normalMappingShader);
        shaders.push_back(parallaxMappingShader);
        shaders.push_back(defaultShader);
        shaders.push_back(cellShading);
        shaders.push_back(AlphaBlendingShader);
        shaders.push_back(BasicTransformShader);
        shaders.push_back(LightModelShader);

    }
    // if there is an error loading any of these Shaders, display error message to user
    catch (std::runtime_error e)  
    {
        gLastError = e.what();
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



    // #############################
    //  load textures
    // #############################

    try 
    {
        characterTexture    = new Texture("StoneDiffuseSpecular.dds");
        patternTexture      = new Texture("PatternDiffuseSpecular.dds");
        paternNormalMap     = new Texture("PatternNormal.dds");
        cobbleTexture       = new Texture("CobbleDiffuseSpecular.dds");
        cobbleNormalMap     = new Texture("CobbleNormalHeight.dds");
        woodTexture         = new Texture("WoodDiffuseSpecular.dds");
        crateTexture        = new Texture("CargoA.dds");
        grassTexture        = new Texture("GrassDiffuseSpecular.dds");
        lightTexture        = new Texture("Flare.jpg");
        alphaTexture        = new Texture("Glass.png");
        trolltexture        = new Texture("Green.png");
        cellMap             = new Texture("CellGradient.png");

        // add the textures to the list of textures
        textures.push_back(characterTexture);
        textures.push_back(paternNormalMap);
        textures.push_back(cobbleTexture);
        textures.push_back(cobbleNormalMap);
        textures.push_back(woodTexture);
        textures.push_back(crateTexture);
        textures.push_back(grassTexture);
        textures.push_back(lightTexture);
        textures.push_back(patternTexture);
    }
    // if there is an error loading any of these meshes, display error message to user
    catch (std::runtime_error e)
    {
        gLastError = e.what();
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


// Prepare the scene
// Returns true on success
bool InitScene()
{

    // #############################
    //  create models
    // #############################

    gTeapot     = new Model(gTeapotMesh);
    gSphere     = new Model(gSphereMesh);
    gCube       = new Model(gCubeMesh);
    gBumpedCube = new Model(gBumpedCubeMesh);
    gCrate      = new Model(gCrateMesh);
    gGround     = new Model(gGroundMesh);
    gTest       = new Model(gCubeMesh);
    gTroll      = new Model(gTrollMesh);


	// Position models
	gTeapot->SetPosition({ 20, 0, 0 });
    gTeapot->SetScale(1); 
    gTeapot->SetRotation({ 0, ToRadians(135.0f), 0 });
    gSphere->SetPosition({ 10,10,10 });
    gTest->SetPosition({ 10,20,10 });
    gCube->SetPosition({ 30,20,10 });
    gBumpedCube->SetPosition({ 60,30,20 });
	gCrate-> SetPosition({ 45, 0, 45 });
	gCrate-> SetScale(6);
    gTroll->SetPosition({ 30, 30, 10 });
    gTroll->SetScale(5);


    // #############################
    //  create Lights
    // #############################


    // create each light
    gLights.push_back(new Light( Light::LightType::Spot,        { 0.8f, 0.8f, 1.0f }, 10));
    gLights.push_back(new Light( Light::LightType::Point,       { 1.0f, 0.8f, 0.2f }, 40));
    gLights.push_back(new Light( Light::LightType::Directional, { 1.0f, 0.1f, 1.0f }, 0.4f)); // directional lights are very bright, so their strength property is a LOT lower compared to other lights

    // position and rotate each light
    gLights[0]->model->SetPosition({  30, 20,  0 });
    gLights[1]->model->SetPosition({ -20, 50, 20 });
    gLights[2]->model->SetPosition({  60, 40, 20 });
    gLights[2]->model->SetRotation({ ToRadians(50.0f), ToRadians(-50.0f), 0.0f });


    // #############################
    //  create camera
    // #############################

    gCamera = new Camera();
    gCamera->SetPosition({ 15, 30,-70 });
    gCamera->SetRotation({ ToRadians(13), 0, 0 });

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
    
    // delete models
    delete gGround;    gGround    = nullptr;
    delete gCrate;     gCrate     = nullptr;
    delete gTeapot;  gTeapot = nullptr;
    delete gSphere; gSphere = nullptr;
    delete gCube; gCube = nullptr;

    // delete meshes
    delete gLightMesh;     gLightMesh     = nullptr;
    delete gGroundMesh;    gGroundMesh    = nullptr;
    delete gCrateMesh;     gCrateMesh     = nullptr;
    delete gTeapotMesh;  gTeapotMesh = nullptr;

    delete gCamera;    gCamera = nullptr;
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
    gD3DContext->VSSetShader(defaultShader->vertexShader, nullptr, 0);
    gD3DContext->PSSetShader(defaultShader->pixelShader,  nullptr, 0);
    
    // States - no blending, normal depth buffer and culling
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullBackState);
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);


    


    // Render other lit models, only change textures for each onee
    gD3DContext->PSSetShaderResources(0, 1, &characterTexture->gTextureMapSRV); 
    gTeapot->Render();

    gD3DContext->PSSetShaderResources(0, 1, &crateTexture->gTextureMapSRV);
    gCrate->Render();





    //sphere
    gD3DContext->VSSetShader(wiggleShader->vertexShader, nullptr, 0);
    gD3DContext->PSSetShader(wiggleShader->pixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &characterTexture->gTextureMapSRV);
    gSphere->Render();

    //cube
    gD3DContext->VSSetShader(fadingShader->vertexShader, nullptr, 0);
    gD3DContext->PSSetShader(fadingShader->pixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &characterTexture->gTextureMapSRV);// pass first texture to shader
    gD3DContext->PSSetShaderResources(1, 1, &woodTexture->gTextureMapSRV);// pass second texture to shader
    gCube->Render();

    //bumped cube
    gD3DContext->VSSetShader(normalMappingShader->vertexShader, nullptr, 0);
    gD3DContext->PSSetShader(normalMappingShader->pixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &patternTexture->gTextureMapSRV);// pass first texture to shader
    gD3DContext->PSSetShaderResources(1, 1, &paternNormalMap->gTextureMapSRV);// pass second texture to shader
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);
    gBumpedCube->Render();


    // paralax mapped ground
    gD3DContext->VSSetShader(parallaxMappingShader->vertexShader, nullptr, 0);
    gD3DContext->PSSetShader(parallaxMappingShader->pixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &cobbleTexture->gTextureMapSRV);// pass first texture to shader
    gD3DContext->PSSetShaderResources(1, 1, &cobbleNormalMap->gTextureMapSRV);// pass second texture to shader
    gGround->Render();


    gD3DContext->VSSetShader(AlphaBlendingShader->vertexShader, nullptr, 0);
    gD3DContext->PSSetShader(AlphaBlendingShader->pixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &alphaTexture->gTextureMapSRV);// pass second texture to shader
    gD3DContext->OMSetBlendState(gAlphaBlendingState, nullptr, 0xffffff);
    gTest->Render();

    //troll cell shading
    // rendering outline - slightly scales object and draws black. No textures needed, draws outline in plain colour
    gD3DContext->VSSetShader(cellShadingOutline->vertexShader, nullptr, 0);
    gD3DContext->PSSetShader(cellShadingOutline->pixelShader, nullptr, 0);
    gD3DContext->OMSetBlendState(gNoBlendingState, nullptr, 0xffffff);
    gD3DContext->OMSetDepthStencilState(gUseDepthBufferState, 0);
    gD3DContext->RSSetState(gCullFrontState);
    gTroll->Render();
    gD3DContext->RSSetState(gCullBackState); // switch back to CullBack for the rest of the scene

    // rendering model:
    gD3DContext->VSSetShader(cellShading->vertexShader, nullptr, 0);
    gD3DContext->PSSetShader(cellShading->pixelShader, nullptr, 0);
    gD3DContext->PSSetShaderResources(0, 1, &trolltexture->gTextureMapSRV); 
    gD3DContext->PSSetSamplers(0, 1, &gAnisotropic4xSampler);
    gD3DContext->PSSetShaderResources(1, 1, &cellMap->gTextureMapSRV); 
    gD3DContext->PSSetSamplers(1, 1, &gPointSampler);
    gTroll->Render();


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



// Rendering the scene
void RenderScene()
{
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
    gPerFrameConstants.cameraPosition = gCamera->Position();
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
    RenderSceneFromCamera(gCamera);


    //// Scene completion ////

    // When drawing to the off-screen back buffer is complete, we "present" the image to the front buffer (the screen)
    // Set first parameter to 1 to lock to vsync (typically 60fps)
    gSwapChain->Present(lockFPS ? 1 : 0, 0);
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

    gLights[1]->strength -= 0.2f * strengthMultiplier; // 40%
    if (gLights[1]->strength <= 0)                    // make one light pulsate on and off
        strengthMultiplier = -1;                      //
    else if (gLights[1]->strength >= 100)             //
        strengthMultiplier = 1;                       //



    gLights[0]->colour.x -= 0.003f  * colourMultiplier; // 40%
    if (gLights[0]->colour.x <= 0)                    // make the other light gradually change colour between blue and white
        colourMultiplier = -1;                        //
    else if (gLights[0]->colour.x >= 1)               //
        colourMultiplier = 1;                         //

}
