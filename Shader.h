//--------------------------------------------------------------------------------------
// Loading GPU shaders
// Creation of constant buffers to help send C++ values to shaders each frame
//--------------------------------------------------------------------------------------
#ifndef _SHADER_H_INCLUDED_
#define _SHADER_H_INCLUDED_

#include "Common.h"

//--------------------------------------------------------------------------------------
// Shader Class
//--------------------------------------------------------------------------------------

class Shader
{
public:
	// load a pixel and vertex shader from a filename
	// _ps and vs file extensions are appended to the filename you provide
	// when boolean set to false, the default ps and vs are loaded instead
	// Example:
	// Shader("AlphaBlending", true, false) will load AlphaBlending_ps with the default vertex shader
	Shader(std::string shaderfileName, bool loadPixelShader = true, bool loadVertexShader = true);


	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;

	bool LoadShaders(std::string shaderfileName, bool loadPixelShader = true, bool loadVertexShader = true);
	void ReleaseShaders();
};


//--------------------------------------------------------------------------------------
// Constant buffer creation / destruction
//--------------------------------------------------------------------------------------

// Create and return a constant buffer of the given size
// The returned pointer needs to be released before quitting. Returns nullptr on failure
ID3D11Buffer* CreateConstantBuffer(int size);


//--------------------------------------------------------------------------------------
// Helper functions
//--------------------------------------------------------------------------------------

// Load a shader, include the file in the project and pass the name (without the .hlsl extension)
// to this function. The returned pointer needs to be released before quitting. Returns nullptr on failure
ID3D11VertexShader* LoadVertexShader(std::string shaderName);
ID3D11PixelShader*  LoadPixelShader (std::string shaderName);

// Helper function. Returns nullptr on failure.
ID3DBlob* CreateSignatureForVertexLayout(const D3D11_INPUT_ELEMENT_DESC vertexLayout[], int numElements);



#endif //_SHADER_H_INCLUDED_
