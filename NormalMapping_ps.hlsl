//--------------------------------------------------------------------------------------
// Texture Pixel Shader
//--------------------------------------------------------------------------------------
// Pixel shader simply samples a diffuse texture map and tints with colours from vertex shadeer

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Textures (texture maps)
//--------------------------------------------------------------------------------------

// Here we allow the shader access to a texture that has been loaded from the C++ side and stored in GPU memory.
// Note that textures are often called maps (because texture mapping describes wrapping a texture round a mesh).
// Get used to people using the word "texture" and "map" interchangably.
Texture2D DiffuseSpecularMap : register(t0); // Diffuse map (main colour) in rgb and specular map (shininess level) in alpha - C++ must load this into slot 0
Texture2D NormalMap : register(t1); // Normal map in rgb - C++ must load this into slot 1
SamplerState TexSampler : register(s0); // A sampler is a filter for a texture like bilinear, trilinear or anisotropic


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

//***| INFO |*********************************************************************************
// Normal mapping pixel shader function. The lighting part of the shader is the same as the
// per-pixel lighting shader - only the source of the surface normal is different
//
// An extra "Normal Map" texture is used - this contains normal (x,y,z) data in place of
// (r,g,b) data indicating the normal of the surface *per-texel*. This allows the lighting
// to take account of bumps on the texture surface. Using these normals is complex:
//    1. We must store a "tangent" vector as well as a normal for each vertex (the tangent
//       is basically the direction of the texture U axis in model space for each vertex)
//    2. Get the (interpolated) model normal and tangent at this pixel from the vertex
//       shader - these are the X and Z axes of "tangent space"
//    3. Use a "cross-product" to calculate the bi-tangent - the missing Y axis
//    4. Form the "tangent matrix" by combining these axes
//    5. Extract the normal from the normal map texture for this pixel
//    6. Use the tangent matrix to transform the texture normal into model space, then
//       use the world matrix to transform it into world space
//    7. This final world-space normal can be used in the usual lighting calculations, and
//       will show the "bumpiness" of the normal map
//
// Note that all this detail boils down to just five extra lines of code here
//********************************************************************************************
float4 main(NormalMappingPixelShaderInput input) : SV_Target
{
    
    
    input.uv += gtimer/5;
	//************************
	// Normal Map Extraction
	//************************

	// Will use the model normal/tangent to calculate matrix for tangent space. The normals for each pixel are *interpolated* from the
	// vertex normals/tangents. This means they will not be length 1, so they need to be renormalised (same as per-pixel lighting issue)
    float3 modelNormal = normalize(input.modelNormal);
    float3 modelTangent = normalize(input.modelTangent);

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space. This is just a matrix built from the three axes (very advanced note - by default shader matrices
	// are stored as columns rather than in rows as in the C++. This means that this matrix is created "transposed" from what we would
	// expect. However, for a 3x3 rotation matrix the transpose is equal to the inverse, which is just what we require)
    float3 modelBiTangent = cross(modelNormal, modelTangent);
    float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent * 3, modelNormal);
	
	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
    float3 textureNormal = 2.0f * NormalMap.Sample(TexSampler, input.uv).rgb - 1.0f; // Scale from 0->1 to -1->1

	// Now convert the texture normal into model space using the inverse tangent matrix, and then convert into world space using the world
	// matrix. Normalise, because of the effects of texture filtering and in case the world matrix contains scaling
    float3 worldNormal = normalize(mul((float3x3) gWorldMatrix, mul(textureNormal, invTangentMatrix)));


	///////////////////////
	// Calculate lighting
	///////////////////////
	
   // Lighting equations
    float3 cameraDirection = normalize(gCameraPosition - input.worldPosition);
	
	
	
    float3 sumOfDiffuse = 0;
    float3 sumOfSpecular = 0;

    for (int i = 0; i < gNumLights; i++)
    {
        float3 lightVector = light[i].lightPosition - input.worldPosition;
        float lightDistance = length(lightVector);
        float3 lightDirection = lightVector / lightDistance; // Quicker than normalising as we have length for attenuation
        float3 DiffuseLight = light[i].lightColour * max(dot(worldNormal, lightDirection), 0) / lightDistance;

        float3 halfway = normalize(lightDirection + cameraDirection);
        float3 SpecularLight = DiffuseLight * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);
		
		
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
            DiffuseLight = spotLight[i].lightColour * max(dot(worldNormal, lightDirection), 0) / lightDistance;

            float3 halfway = normalize(lightDirection + cameraDirection);
            SpecularLight = DiffuseLight * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);
        }
        sumOfDiffuse += DiffuseLight;
        sumOfSpecular += SpecularLight;
    }
    
	// Sum the effect of the lights - add the ambient at this stage rather than for each light (or we will get too much ambient)
    //float3 diffuseLight = gAmbientColour + sumOfDiffuse;
    //float3 specularLight = sumOfSpecular;

   // // Light 1
   // float3 light1Vector = gLight1Position - input.worldPosition;
   // float light1Distance = length(light1Vector);
   // float3 light1Direction = light1Vector / light1Distance; // Quicker than normalising as we have length for attenuation
   // float3 diffuseLight1 = gLight1Colour * max(dot(worldNormal, light1Direction), 0) / light1Distance;

   // float3 halfway = normalize(light1Direction + cameraDirection);
   // float3 specularLight1 = diffuseLight1 * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);


   // // Light 2
   // float3 light2Vector = gLight2Position - input.worldPosition;
   // float light2Distance = length(light2Vector);
   // float3 light2Direction = light2Vector / light2Distance;
   // float3 diffuseLight2 = gLight2Colour * max(dot(worldNormal, light2Direction), 0) / light2Distance;

   // halfway = normalize(light2Direction + cameraDirection);
   // float3 specularLight2 = diffuseLight2 * pow(max(dot(worldNormal, halfway), 0), gSpecularPower);



    // Sample diffuse material colour for this pixel from a texture using a given sampler that you set up in the C++ code
    // Ignoring any alpha in the texture, just reading RGB
    float4 textureColour = DiffuseSpecularMap.Sample(TexSampler, input.uv);
    float3 diffuseMaterialColour = textureColour.rgb;
    float specularMaterialColour = textureColour.a;

    float3 finalColour = (gAmbientColour + sumOfDiffuse) * diffuseMaterialColour +
                         (sumOfSpecular) * specularMaterialColour;

    return float4(finalColour, 1.0f); // Always use 1.0f for alpha - no alpha blending in this lab
}