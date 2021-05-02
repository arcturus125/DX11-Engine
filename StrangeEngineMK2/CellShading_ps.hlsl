//--------------------------------------------------------------------------------------
// Vertex shader for cell shading
//--------------------------------------------------------------------------------------

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.
Texture2D    DiffuseMap : register(t0); // Diffuse map only
Texture2D    CellMap    : register(t1); // CellMap is a 1D map that is used to limit the range of colours used in cell shading

SamplerState TexSampler       : register(s0); // Sampler for use on textures
SamplerState PointSampleClamp : register(s1); // No filtering of cell maps (otherwise the cell edges would be blurred)


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Pixel shader entry point - each shader has a "main" function
// This shader just samples a diffuse texture map
float4 main(LightingPixelShaderInput input) : SV_Target
{
    // Lighting equations
    input.worldNormal = normalize(input.worldNormal); // Normal might have been scaled by model scaling or interpolation so renormalise

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
        
        //****| INFO |*************************************************************************************//
	    // To make a cartoon look to the lighting, we clamp the basic light level to just a small range of
	    // colours. This is done by using the light level itself as the U texture coordinate to look up
	    // a colour in a special 1D texture (a single line). This could be done with if statements, but
	    // GPUs are much faster at looking up small textures than if statements
	    //*************************************************************************************************//
        float diffuseLevel1 = max(dot(input.worldNormal, lightDirection), 0);
        float cellDiffuseLevel1 = CellMap.Sample(PointSampleClamp, diffuseLevel1).r;
        DiffuseLight = light[i].lightColour * cellDiffuseLevel1 / lightDistance;

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
            float3 DiffuseLight = light[i].lightColour * max(dot(input.worldNormal, lightDirection), 0) / lightDistance;
        
            //****| INFO |*************************************************************************************//
	        // To make a cartoon look to the lighting, we clamp the basic light level to just a small range of
	        // colours. This is done by using the light level itself as the U texture coordinate to look up
	        // a colour in a special 1D texture (a single line). This could be done with if statements, but
	        // GPUs are much faster at looking up small textures than if statements
	        //*************************************************************************************************//
            float diffuseLevel1 = max(dot(input.worldNormal, lightDirection), 0);
            float cellDiffuseLevel1 = CellMap.Sample(PointSampleClamp, diffuseLevel1).r;
            DiffuseLight = light[i].lightColour * cellDiffuseLevel1 / lightDistance;

            float3 halfway = normalize(lightDirection + cameraDirection);
            float3 SpecularLight = DiffuseLight * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower);
        }
        sumOfDiffuse += DiffuseLight;
        sumOfSpecular += SpecularLight;
    }
    // directional lights
    for (int i = 0; i < gNumDirectionalLights; i++)
    {
        float3 lightVector = directionalLight[i].lightPosition - input.worldPosition;
        float3 lightDirection = -directionalLight[i].lightFacing; //lightVector / lightDistance; // Quicker than normalising as we have length for attenuation
        float3 DiffuseLight = directionalLight[i].lightColour * max(dot(input.worldNormal, lightDirection), 0);
        
        //****| INFO |*************************************************************************************//
	    // To make a cartoon look to the lighting, we clamp the basic light level to just a small range of
	    // colours. This is done by using the light level itself as the U texture coordinate to look up
	    // a colour in a special 1D texture (a single line). This could be done with if statements, but
	    // GPUs are much faster at looking up small textures than if statements
	    //*************************************************************************************************//
        float diffuseLevel1 = max(dot(input.worldNormal, lightDirection), 0);
        float cellDiffuseLevel1 = CellMap.Sample(PointSampleClamp, diffuseLevel1).r;
        DiffuseLight = directionalLight[i].lightColour * cellDiffuseLevel1 * 3;

        float3 halfway = normalize(lightDirection + cameraDirection);
        float3 SpecularLight = DiffuseLight * pow(max(dot(input.worldNormal, halfway), 0), gSpecularPower);
		
		
        sumOfDiffuse += DiffuseLight;
        sumOfSpecular += SpecularLight;
    }
    
	// Sum the effect of the lights - add the ambient at this stage rather than for each light (or we will get too much ambient)
    float3 diffuseLight = gAmbientColour + sumOfDiffuse;
    float3 specularLight = sumOfSpecular;

   

    
    // Sample diffuse material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    // Ignoring any alpha in the texture, just reading RGB
    float4 textureColour = DiffuseMap.Sample(TexSampler, input.uv);
    float3 diffuseMaterialColour = textureColour.rgb;
    float specularMaterialColour = textureColour.a;

    float3 finalColour = (gAmbientColour + diffuseLight) * diffuseMaterialColour +
                         (specularLight) * specularMaterialColour;

    return float4(finalColour, 1.0f); // Always use 1.0f for alpha - no alpha blending in this lab
}