#pragma once

#include "CVector3.h"
#include "Model.h"
class Light
{
public:
    Model* model;
    CVector3 colour;
    float    strength;
};

