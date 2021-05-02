
#include "pch.h"
#include "Light.h"


#include "CVector3.h"
#include "Model.h"
#include "Mesh.h"

Light::Light(LightType pType, CVector3 pColour, float pStrength)
{
    gLightMesh = new Mesh("Light.x");
    model = new Model(gLightMesh);
    type = pType;
    colour = pColour;
    strength = pStrength;


    model->SetScale(pow(strength, 0.7f)); // stronger light = bigger model
}

Light::PointLightData Light::GetPointLightData()
{
    PointLightData data;
    data.lightColour = colour * strength;
    data.lightPosition = model->Position();
    return data;
}

Light::SpotLightData Light::GetSpotLightData()
{
    SpotLightData data;
    data.lightColour = colour * strength;
    data.lightPosition = model->Position();
    data.lightFacing = Normalise(model->WorldMatrix().GetZAxis());    // Additional lighting information for spotlights
    data.lightCosHalfAngle = cos(ToRadians(SpotLight_ConeAngle / 2)); // --"--
    return data;
}

Light::DirectionalLightData Light::GetDirectionalLightData()
{
    DirectionalLightData data;
    data.lightColour = colour * strength;
    data.lightPosition = model->Position();
    data.lightFacing = Normalise(model->WorldMatrix().GetZAxis());    // Additional lighting information for Directional lights
    return data;
}
