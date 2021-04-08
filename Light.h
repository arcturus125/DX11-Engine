//#pragma once

#ifndef _LIGHT_H_INCLUDED_
#define _LIGHT_H_INCLUDED_


//#include "Model.h"

#include "CVector3.h"
class Model;

class Light
{
public:
    struct LightingData
    {
        CVector3   lightPosition;
        float      padding1;      
        CVector3   lightColour;
        float      padding2;
    };

    Model* model;
    CVector3 colour;
    float    strength;

    LightingData getLightingData();


};

#endif //_LIGHT_H_INCLUDED_

