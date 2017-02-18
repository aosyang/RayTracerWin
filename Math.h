#pragma once

#include <stdlib.h>
#include "RVector.h"

// [0, 1] random
inline float Random()
{
	return (float)rand() / RAND_MAX;
}

// Generate random hemisphere direction
RVec3 RandomHemisphereDir(const RVec3& dir);
