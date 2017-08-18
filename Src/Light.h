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
	union
	{
		RVec3	Position;		// For point light
		RVec3	Direction;		// For direction light
	};
	RVec3		Color;
};
