#include <iostream>
#include "Engine.h" // <----including engine here

int main()
{
    std::cout << "Hello World!\n";

    // set tehe default media folder
    AddShaderFolder("C:\\StrangeEngine\\Debug\\"); // path to the compiled shaders
    AddMediaFolder("C:\\StrangeEngine\\Media\\"); // path to other media
    
    // create and run the engine
    StartEngine(GetModuleHandle(0));

    // scene setup here

    // while engine.isRunning

    // scene updates here
}

