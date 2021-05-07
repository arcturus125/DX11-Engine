//#pragma once

#ifndef _LIGHT_H_INCLUDED_
#define _LIGHT_H_INCLUDED_


//#include "Model.h"

#include "CVector3.h"
class Model;
class Mesh;

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
        CVector3   lightFacing;
        float      lightCosHalfAngle;
    };
    struct DirectionalLightData
    {
        CVector3   lightPosition;
        float      padding1;
        CVector3   lightColour;
        float      padding2;
        CVector3   lightFacing;
        float      padding3;
    };

    enum class LightType
    {
        Point,
        Spot,
        Directional
    };

    Mesh* gLightMesh;
    LightType type;
    Model* model;
    CVector3 colour;
    float    strength;
   
    // spot light attributes
    float SpotLight_ConeAngle = 90.0f;

    __declspec(dllexport) Light(LightType pType,CVector3 pColour, float strength);

    PointLightData GetPointLightData();
    SpotLightData GetSpotLightData();
    DirectionalLightData GetDirectionalLightData();


};

#endif //_LIGHT_H_INCLUDED_

