#include "Light.h"


#include "CVector3.h"
#include "Model.h"

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
