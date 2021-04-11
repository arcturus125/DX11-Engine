//#pragma once

#ifndef _LIGHT_H_INCLUDED_
#define _LIGHT_H_INCLUDED_


//#include "Model.h"

#include "CVector3.h"
class Model;

class Light
{
public:
    struct PointLightData
    {
        CVector3   lightPosition;
        float      padding1;      
        CVector3   lightColour;
        float      padding2;
    };
    struct SpotLightData
    {
        CVector3   lightPosition;
        float      padding1;
        CVector3   lightColour;
        float      padding2;
        CVector3 lightFacing;
        float lightCosHalfAngle;
    };

    enum class LightType
    {
        Point,
        Spot
    };

    LightType type;
    Model* model;
    CVector3 colour;
    float    strength;
   
    // spot light attributes
    float SpotLight_ConeAngle = 90.0f;

    PointLightData GetPointLightData();
    SpotLightData GetSpotLightData();


};

#endif //_LIGHT_H_INCLUDED_

