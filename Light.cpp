#include "Light.h"


#include "CVector3.h"
#include "Model.h"

Light::LightingData Light::getLightingData()
{
    LightingData data;
    data.lightColour = colour * strength;
    data.lightPosition = model->Position();
    return data;
}
