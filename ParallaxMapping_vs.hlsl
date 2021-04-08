//--------------------------------------------------------------------------------------
// Basic Transformation and Lighting Vertex Shader
//--------------------------------------------------------------------------------------

#include "Common.hlsli" // Shaders can also use include files - note the extension


//--------------------------------------------------------------------------------------
// Shader code
//--------------------------------------------------------------------------------------

//****| INFO |*******************************************************************************************//
// The vertex shader for parallax mapping is identical to normal mapping. The parallax adjustment occurs
// in the pixel shader
//*******************************************************************************************************//
NormalMappingPixelShaderInput main(TangentVertex modelVertex)
{
    NormalMappingPixelShaderInput output; // This is the data the pixel shader requires from this vertex shader

    // Input position is x,y,z only - need a 4th element to multiply by a 4x4 matrix. Use 1 for a point (0 for a vector) - recall lectures
    float4 modelPosition = float4(modelVertex.position, 1); 

    // Multiply by the world matrix passed from C++ to transform the model vertex position into world space. 
    // In a similar way use the view matrix to transform the vertex from world space into view space (camera's point of view)
    // and then use the projection matrix to transform the vertex to 2D projection space (project onto the 2D screen)
    float4 worldPosition     = mul(gWorldMatrix,      modelPosition);
    float4 viewPosition      = mul(gViewMatrix,       worldPosition);
    output.projectedPosition = mul(gProjectionMatrix, viewPosition);

    output.worldPosition = worldPosition.xyz; // Also pass world position to pixel shader for lighting

	// Unlike the position, send the model's normal and tangent untransformed (in model space). The pixel shader will do the matrix work on normals
	output.modelNormal  = modelVertex.normal;
	output.modelTangent = modelVertex.tangent;

    // Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
    output.uv = modelVertex.uv;

    return output; // Ouput data sent down the pipeline (to the pixel shader)
}
