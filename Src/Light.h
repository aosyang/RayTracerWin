#pragma once

#include "RVector.h"

enum LightType
{
	LT_Point,
	LT_Directional,
};

struct Light
{
	LightType type;
	RVec3 pos_dir;
	RVec3 color;
};