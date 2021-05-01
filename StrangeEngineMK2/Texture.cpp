
#include "pch.h"
#include "Texture.h"
#include <GraphicsHelpers.h>
#include <stdexcept>

Texture::Texture(std::string filePath)
{
	if (!LoadTexture(filePath, &gTextureMap, &gTextureMapSRV))
	{
		throw std::runtime_error("Error Loading Texture:" + filePath);
	}
}
