#include <iostream>
#include "Engine.h" // <----including engine here



Camera* myCamera;

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

    myCamera = new Camera();
    myCamera->SetPosition({ 15, 30,-70 });
    myCamera->SetRotation({ ToRadians(13), 0, 0 });

    SetMainCamera(myCamera);
}

void Update(float frametime)
{
    std::cout << "Update! " << frametime <<"\n";

    myCamera->Control(frametime, Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D);
}

void End()
{
    delete myCamera;    myCamera = nullptr;
}

