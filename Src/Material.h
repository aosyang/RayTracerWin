//=============================================================================
// Material.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RVector.h"

struct RMaterial
{
	RMaterial()
		: Color(1.0f, 1.0f, 1.0f)
		, UseCheckerboardPattern(false)
		, MaterialFlags(0)
	{}

	RMaterial(RVec3 InColor, bool bCheckerboard, int Flags)
		: Color(InColor)
		, UseCheckerboardPattern(bCheckerboard)
		, MaterialFlags(Flags)
	{}

	RVec3	Color;
	bool	UseCheckerboardPattern;
	int		MaterialFlags;
};
