#include <iostream>
#include "Engine.h" // <----including engine here


//DEFAULT
Camera* myCamera;
Light* directionalLight;

//CUSTOM
Light* spotLight;
Light* pointLight;


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

    //// position lights
    spotLight->model->SetPosition({ 30, 20,  0 });
    pointLight->model->SetPosition({ -20, 50, 20 });
    directionalLight->model->SetPosition({ 60, 40, 20 });
    directionalLight->model->SetRotation({ ToRadians(50.0f), ToRadians(-50.0f), 0.0f });
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

