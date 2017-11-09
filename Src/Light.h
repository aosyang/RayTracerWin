//=============================================================================
// Light.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RVector.h"

enum ELightType
{
	LT_Point,
	LT_Directional,
};

struct LightData
{
	ELightType	Type;
	RVec3		PositionOrDirection;		// Position for point light; Direction for directional light
	RVec3		Color;
};
