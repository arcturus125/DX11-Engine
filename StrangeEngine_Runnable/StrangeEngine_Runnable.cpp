#include <iostream>
#include "Engine.h" // <----including engine here


//DEFAULT
Camera* myCamera;
Light* directionalLight;

//CUSTOM
Light* spotLight;
Light* pointLight;
Texture* characterTexture = nullptr;
Texture* woodTexture = nullptr;
Texture* crateTexture = nullptr;
Texture* grassTexture = nullptr;
Texture* lightTexture = nullptr;
Texture* patternTexture = nullptr;
Texture* paternNormalMap = nullptr;
Texture* cobbleTexture = nullptr;
Texture* cobbleNormalMap = nullptr;
Texture* alphaTexture = nullptr;
Texture* trolltexture = nullptr;
Texture* cellMap = nullptr;
Shader* wiggleShader = nullptr;
Shader* fadingShader = nullptr;
Shader* normalMappingShader = nullptr;
Shader* parallaxMappingShader = nullptr;
Shader* cellShading = nullptr;
Shader* cellShadingOutline = nullptr;
Shader* AlphaBlendingShader = nullptr;
Shader* BasicTransformShader = nullptr;
Shader* LightModelShader = nullptr;
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


int strengthMultiplier = 1;
int colourMultiplier = 1;

// Variables controlling light orbiting of the cube
const float gLightOrbit = 20.0f;
const float gLightOrbitSpeed = 0.7f;

void Start();
void Update(float frametime);
void End();

int main()
{
    std::cout << "Hello World!\n";

    // set the default media folder
    AddShaderFolder("C:\\StrangeEngine\\Debug\\"); // path to the compiled shaders
    AddMediaFolder("C:\\StrangeEngine\\Media\\"); // path to other media
    
    // create and run the engine
    StartEngine(GetModuleHandle(0), Start, Update, End);
}

void Start()
{
    std::cout << "Start!\n";

    // create the camera
    myCamera = new Camera({ 15, 30,-70 }, { ToRadians(13), 0, 0 });
    SetMainCamera(myCamera); // tell the engine to use the camera we just made

    // create lights
    directionalLight =  new Light(Light::LightType::Directional, { 1.0f, 0.1f, 1.0f }, 0.4f); // directional lights are very bright, so their strength property is a LOT lower compared to other lights
    spotLight =  new Light(Light::LightType::Spot, { 0.8f, 0.8f, 1.0f }, 10);
    pointLight =  new Light(Light::LightType::Point, { 1.0f, 0.8f, 0.2f }, 40);

    // position lights
    spotLight->model->SetPosition({ 30, 20,  0 });
    pointLight->model->SetPosition({ -20, 50, 20 });
    directionalLight->model->SetPosition({ 60, 40, 20 });
    directionalLight->model->SetRotation({ ToRadians(50.0f), ToRadians(-50.0f), 0.0f });


    characterTexture = new Texture("StoneDiffuseSpecular.dds");
    patternTexture = new Texture("PatternDiffuseSpecular.dds");
    paternNormalMap = new Texture("PatternNormal.dds");
    cobbleTexture = new Texture("CobbleDiffuseSpecular.dds");
    cobbleNormalMap = new Texture("CobbleNormalHeight.dds");
    woodTexture = new Texture("WoodDiffuseSpecular.dds");
    crateTexture = new Texture("CargoA.dds");
    grassTexture = new Texture("GrassDiffuseSpecular.dds");
    alphaTexture = new Texture("Glass.png");
    trolltexture = new Texture("Green.png");
    cellMap = new Texture("CellGradient.png");


    wiggleShader = new Shader("Wiggle");
    fadingShader = new Shader("Fading");
    normalMappingShader = new Shader("NormalMapping");
    parallaxMappingShader = new Shader("ParallaxMapping");
    cellShading = new Shader("CellShading");
    cellShadingOutline = new Shader("CellShadingOutline");

    AlphaBlendingShader = new Shader("AlphaBlending", true, false);
    BasicTransformShader = new Shader("BasicTransform", false, true);
    LightModelShader = new Shader("LightModel", true, false);


    gTeapotMesh = new Mesh("teapot.x");
    gSphereMesh = new Mesh("Sphere.x");
    gCubeMesh = new Mesh("Cube.x");
    gTrollMesh = new Mesh("Troll.x");
    gBumpedCubeMesh = new Mesh("Cube.x", true);     // <-----   use true to make this mesh generate tangents
                                                    //          this means that the model will use TangentVertex in common.hlsli instead of
                                                    //          BasicVertex. (meaning that normal maps can now be used on the any model using this mesh)
    gCrateMesh = new Mesh("CargoContainer.x");
    gGroundMesh = new Mesh("Hills.x", true);


    gTeapot = new Model(gTeapotMesh);
    gSphere = new Model(gSphereMesh);
    gCube = new Model(gCubeMesh);
    gBumpedCube = new Model(gBumpedCubeMesh);
    gCrate = new Model(gCrateMesh);
    gGround = new Model(gGroundMesh);
    gTest = new Model(gCubeMesh);
    gTroll = new Model(gTrollMesh);

    // Position models
    gTeapot->SetPosition({ 20, 0, 0 });
    gTeapot->SetScale(1);
    gTeapot->SetRotation({ 0, ToRadians(135.0f), 0 });
    gTeapot->AddTexture(characterTexture);

    gSphere->SetPosition({ 10,10,10 });
    gSphere->SetShader(wiggleShader);
    gSphere->AddTexture(characterTexture);

    gTest->SetPosition({ 10,20,10 });
    gTest->SetShader(AlphaBlendingShader);
    gTest->AddTexture(alphaTexture);
    gTest->SetBlendingState(BlendingState::Alpha);

    gCube->SetPosition({ 30,20,10 });
    gCube->SetShader(fadingShader);
    gCube->AddTexture(characterTexture);
    gCube->AddTexture(woodTexture);

    gBumpedCube->SetPosition({ 60,30,20 });
    gBumpedCube->SetShader(normalMappingShader);
    gBumpedCube->AddTexture(patternTexture);
    gBumpedCube->AddTexture(paternNormalMap);

    gCrate->SetPosition({ 45, 0, 45 });
    gCrate->SetScale(6);
    gCrate->AddTexture(crateTexture);


    // /*
    gTroll->SetPosition({ 30, 30, 10 });
    gTroll->SetScale(5);

    gTroll->SetShader(cellShadingOutline);
    gTroll->SetBlendingState(BlendingState::NoBlending);
    gTroll->SetDepthBufferState(DepthBufferState::UseDepthBuffer);
    gTroll->SetCullingState(CullingState::Front);

    gTroll->AddRendererPass();
    gTroll->SetShader(cellShading, 1);
    gTroll->AddTexture(trolltexture, 1);
    gTroll->AddTexture(cellMap, 1);
    gTroll->SetSampler(SamplerState::Anisotropic4x, 1);
    gTroll->AddSampler(SamplerState::Point, 1);
    gTroll->SetCullingState(CullingState::Back, 1);
    gTroll->SetDepthBufferState(DepthBufferState::UseDepthBuffer, 1);
    gTroll->SetBlendingState(BlendingState::NoBlending, 1);

    // only the first depth buffer that is auto-genereted will load default shaders, textures, samplers, culling states, and depth buffer states
    // TODO: make the AddRenderPass() function create a new pass with defaults loaded
    //*/


    gGround->SetShader(parallaxMappingShader);
    gGround->AddTexture(cobbleTexture);
    gGround->AddTexture(cobbleNormalMap);




}

void Update(float frametime)
{
    std::cout << "Update! " << frametime <<"\n";

    // WASD keys to move, arrow keys to look around
    myCamera->Control(frametime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D);
    


    pointLight->strength -= 0.2f * strengthMultiplier; // 40%
    if (pointLight->strength <= 0)                    // make one light pulsate on and off
        strengthMultiplier = -1;                      //
    else if (pointLight->strength >= 100)             //
        strengthMultiplier = 1;                       //



    spotLight->colour.x -= 0.003f * colourMultiplier; // 40%
    if (spotLight->colour.x <= 0)                    // make the other light gradually change colour between blue and white
        colourMultiplier = -1;                        //
    else if (spotLight->colour.x >= 1)               //
        colourMultiplier = 1;                         //

    // Orbit the light - a bit of a cheat with the static variable [ask the tutor if you want to know what this is]
    static float rotate = 0.0f;
    static bool go = true;
    spotLight->model->SetPosition(CVector3{20, 0, 0 } + CVector3{ cos(rotate) * gLightOrbit, 10, sin(rotate) * gLightOrbit });
    if (go)  rotate -= gLightOrbitSpeed * frametime;
    if (KeyHit(Key_1))  go = !go;
}

// to avoid memory leaks, delete any dynamically created objects here
// if you are gettign errors, you are deleting something that the engine will delete for you
void End()
{
    delete myCamera;    myCamera = nullptr;
}

