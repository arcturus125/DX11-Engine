
#include <iostream>
#include "StrangeEngine.h"
#include <Input.h>

void Start();
void Update();
void End();

int main()
{
    std::cout << "Hello World!\n";
    StrangeEngine strange;
    strange.StartEngine(Start,Update,End);
}

void Start()
{}
void Update()
{
    if (KeyHit(Key_A))
    {
        std::cout << "Key Hit!\n";
    }
}
void End()
{}
