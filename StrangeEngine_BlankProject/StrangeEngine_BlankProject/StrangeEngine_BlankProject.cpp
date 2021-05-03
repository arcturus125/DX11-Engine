#include <iostream>
#include "Engine.h" // <----including engine here

void Start();
void Update(float frametime);

int main()
{
    std::cout << "Hello World!\n";

    // set the default media folder
    AddShaderFolder("C:\\StrangeEngine\\Debug\\"); // path to the compiled shaders
    AddMediaFolder("C:\\StrangeEngine\\Media\\"); // path to other media

    // create and run the engine
    StartEngine(GetModuleHandle(0), Start, Update);
}

void Start()
{
    std::cout << "Start!\n";
}

void Update(float frametime)
{
    std::cout << "Update! " << frametime << "\n";
}

