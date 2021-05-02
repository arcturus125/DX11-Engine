//--------------------------------------------------------------------------------------
// Per-Pixel Lighting Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader receives position and normal from the vertex shader and uses them to calculate
// lighting per pixel. Also samples a samples a diffuse + specular texture map and combines with light colour.

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.
Texture2D DiffuseSpecularMap : register(t0); // Textures here can contain a diffuse map (main colour) in their rgb channels and a specular map (shininess) in the a channel
SamplerState TexSampler      : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic - this is the sampler used for the texture above


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader entry point - each shader has a "main" function
// This shader just samples a diffuse texture map
float4 main(LightingPixelShaderInput input) : SV_Target
{
    // Normal might have been scaled by model scaling or interpolation so renormalise
    input.worldNormal = normalize(input.worldNormal); 
    input.uv += gtimer;

	///////////////////////
	// Calculate lighting
	///////////////////////
    
    // Direction from pixel to camera
    float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);

	
	
    float3 sumOfDiffuse = 0;
    float3 sumOfSpecular = 0;

    for (int i = 0; i < gNumLights; i++)
    {
        float3 lightVector = light[i].lightPosition - input.worldPosition;
        float lightDistance = length(lightVector);
        float3 lightDirection = lightVector / lightDistance; // Quicker than normalising as we have length for attenuation
        float3 DiffuseLight = light[i].lightColour * max(dot(input.worldNormal, lightDirection), 0) / lightDistance;

        float3 halfway = normalize(lightDirection + cameraDirection);
        float3 SpecularLight = DiffuseLight * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower);
		
		
        sumOfDiffuse += DiffuseLight;
        sumOfSpecular += SpecularLight;
    }
    for (int i = 0; i < gNumSpotLights; i++)
    {
        float3 DiffuseLight = 0;
        float3 SpecularLight = 0;
		
		
		// Direction from pixel to light
        float3 lightDirection = normalize(spotLight[i].lightPosition - input.worldPosition);
		
        if (dot(spotLight[i].lightFacing, -lightDirection) > cos(spotLight[i].lightCosHalfAngle)) // check if pixel is within the cone of the spot light
        {
            float3 lightVector = light[i].lightPosition - input.worldPosition;
            float lightDistance = length(lightVector);
            float3 lightDirection = lightVector / lightDistance; // Quicker than normalising as we have length for attenuation
            DiffuseLight = spotLight[i].lightColour * max(dot(input.worldNormal, lightDirection), 0) / lightDistance;

            float3 halfway = normalize(lightDirection + cameraDirection);
            SpecularLight = DiffuseLight * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower);
        }
        sumOfDiffuse += DiffuseLight;
        sumOfSpecular += SpecularLight;
    }
    // directional lights
    for (int i = 0; i < gNumDirectionalLights; i++)
    {
        float3 lightVector = directionalLight[i].lightPosition - input.worldPosition;
        float3 lightDirection = -directionalLight[i].lightFacing; //lightVector / lightDistance; // Quicker than normalising as we have length for attenuation

        float3 DiffuseLight = directionalLight[i].lightColour * max(dot(input.worldNormal, lightDirection), 0) ;

        float3 halfway = normalize(lightDirection + cameraDirection);
        float3 SpecularLight = DiffuseLight * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower);
		
		
        sumOfDiffuse += DiffuseLight;
        sumOfSpecular += SpecularLight;
    }
    
	// Sum the effect of the lights - add the ambient at this stage rather than for each light (or we will get too much ambient)
    float3 diffuseLight = gAmbientColour + sumOfDiffuse;
    float3 specularLight = sumOfSpecular;


	////////////////////
	// Combine lighting and textures

    // Sample diffuse material and specular material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
    float3 diffuseMaterialColour = textureColour.rgb; // Diffuse material colour in texture RGB (base colour of model)
    float specularMaterialColour = textureColour.a;   // Specular material colour in texture A (shininess of the surface)
    

    // Combine lighting with texture colours
    float3 tint = { 0.0f, 1.0f, 0.0f };
    float3 finalColour = diffuseLight * tint * diffuseMaterialColour + specularLight * specularMaterialColour;

    return float4(finalColour, 1.0f); // Always use 1.0f for output alpha - no alpha blending in this lab
}