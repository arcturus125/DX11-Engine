//--------------------------------------------------------------------------------------
// Pixel shader for drawing outlines in cell shading
//--------------------------------------------------------------------------------------

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Just returns a fixed outline colour
float4 main(LightingPixelShaderInput input) : SV_Target
{
    return float4(gOutlineColour, 1.0f); // Always use 1.0f for alpha - no alpha blending in this lab
}