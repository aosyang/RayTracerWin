#pragma once

#include <stdlib.h>
#include "RVector.h"

namespace RMath
{
	// [0, 1] random
	inline float Random()
	{
		return (float)rand() / RAND_MAX;
	}

	// Generate random hemisphere direction with given normal direction
	RVec3 RandomHemisphereDirection(const RVec3& Normal);
}
