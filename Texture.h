#pragma once
#include <string>

// forward declare  the classes to break any circular dependency
class ID3D11Resource;
class ID3D11ShaderResourceView;

class Texture
{
public:

	ID3D11Resource* gTextureMap = nullptr;				// This object represents the memory used by the texture on the GPU
	ID3D11ShaderResourceView* gTextureMapSRV = nullptr; // This object is used to give shaders access to the texture above (SRV = shader resource view)

	Texture(std::string filePath);
};

