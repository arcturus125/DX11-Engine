//--------------------------------------------------------------------------------------
// Vertex shader for drawing outlines in cell shading
//--------------------------------------------------------------------------------------

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

// Vertex shader expands the model slightly for the outline
// Only output is the vertex position transformed into screen space, but needs a little work
// to get the amount to expand based on distance
LightingPixelShaderInput main(BasicVertex modelVertex)
{
    LightingPixelShaderInput output; // This is the data the pixel shader requires from this vertex shader

    // Transform model vertex position to world space using the world matrix passed from C++
    float4 modelPosition = float4(modelVertex.position, 1); 
    float4 worldPosition = mul(gWorldMatrix, modelPosition);

	// Next the usual transform from world space to camera space - but we don't go any further here - this will be used to help expand the outline
	// The result "viewPosition" is the xyz position of the vertex as seen from the camera. The z component is the distance from the camera - that's useful...
    float4 viewPosition = mul(gViewMatrix, worldPosition);

	// Transform model normal to world space. We will use the normal to expand the geometry, not for lighting
	float4 modelNormal = float4(modelVertex.normal, 0.0f); // Set 4th element to 0.0 this time as normals are vectors
	float4 worldNormal = normalize( mul(gWorldMatrix, modelNormal) ); // Normalise in case of world matrix scaling

	// Now we return to the world position of this vertex and expand it along the world normal - that will expand the geometry outwards.
	// Use the distance from the camera to decide how much to expand. Use this distance together with a sqrt to creates an outline that
	// gets thinner in the distance, but always remains clear. Overall thickness is also controlled by the constant "OutlineThickness"
	worldPosition += gOutlineThickness * sqrt(viewPosition.z) * worldNormal;

    // Transform new expanded world-space vertex position to view space then 2D projection space and output
    viewPosition             = mul(gViewMatrix,       worldPosition);
    output.projectedPosition = mul(gProjectionMatrix, viewPosition);

    return output; // Ouput data sent down the pipeline (to the pixel shader)
}
