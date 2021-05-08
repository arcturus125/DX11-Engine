
#include "pch.h"
#include "Texture.h"
#include <GraphicsHelpers.h>
#include <stdexcept>

Texture::Texture(std::string filePath)
{
	for(int i = 0 ;i < mediaPaths.size();i++)
	{
		std::string absoloutePath = mediaPaths[i] + filePath;
		if (LoadTexture(absoloutePath, &gTextureMap, &gTextureMapSRV))
		{
			textures.push_back(this);
			return;
		}
	}

	throw std::runtime_error("Error Loading Texture:" + filePath);
}
