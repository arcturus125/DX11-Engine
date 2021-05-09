#include <iostream>
#include "Engine.h" // <----including engine here


// Global declarations of all game objects
Camera* myCamera;
Light* directionalLight;
Mesh* platformMesh;
Model* platform;
Texture* wood;

// forward declaration of functions
void Start();
void Update(float frametime);
void End();

int main()
{
    // tell the engine where to load media and files from
    AddShaderFolder("C:\\StrangeEngine\\Debug\\"); // path to the compiled shaders
    AddMediaFolder("C:\\StrangeEngine\\Media\\"); // path to other media

    // create and run the engine
    StartEngine(GetModuleHandle(0), Start, Update, End);
}

// run once at the start of the game
void Start()
{
    // create the camera
    myCamera = new Camera({ 15, 30,-70 }, { ToRadians(13), 0, 0 });
    SetMainCamera(myCamera); // tell the engine to use the camera we just made

    // create a directional light
    directionalLight = new Light(Light::LightType::Directional, { 255/255.f, 250/255.f, 112/255.f }, 0.4f);
    directionalLight->model->SetPosition({ 60, 40, 20 });                                 // set the position of the light
    directionalLight->model->SetRotation({ ToRadians(50.0f), ToRadians(-50.0f), 0.0f });  // set to rotation of the light

    wood = new Texture("wood2.jpg"); // load texture from file

    platformMesh = new Mesh("Floor.x"); // load mesh from file
    platform = new Model(platformMesh); // create a GameObject
    platform->AddTexture(wood);         // set the texture on the floor
}

// run every frame
void Update(float frametime)
{
    // WASD keys to move, arrow keys to look around
    myCamera->Control(frametime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D);
}

// run once at the end of the game
void End()
{ /*delete any dynamically created objects here*/ }

