//=============================================================================
// Texture.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Platform.h"
#include "RVector.h"

#include <memory>
#include <string>
#include <vector>

class RTexture
{
public:
	RTexture();

	/// Sample a color by texture coordinate
	RVec4 Sample(float u, float v);

	/// Load a texture from png
	static std::unique_ptr<RTexture> LoadTexturePNG(const std::string& Filename);
    
    static bool SaveBufferToPNG(const std::string& Filename, const UINT32* Pixels, int width, int height);

private:
	std::vector<RVec4>	Pixels;
	int	Width;
	int Height;
};
