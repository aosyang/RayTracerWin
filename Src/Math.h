//=============================================================================
// Math.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
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
    
    // Generate a random vector uniformly spread on a unit sphere
    inline RVec3 RandomUnitVector()
    {
        float t1 = 2.0f * PI * Random();
        float t2 = acosf(1.0f - 2.0f * Random());
        float sin_t2 = sinf(t2);
        return RVec3(sinf(t1) * sin_t2, cosf(t1) * sin_t2, cosf(t2));
    }

	void InitPseudoRandomUnitVector();
	RVec3 PseudoRandomUnitVector();

	// Generate random hemisphere direction with given normal direction
	RVec3 RandomHemisphereDirection(const RVec3& Normal);
}
