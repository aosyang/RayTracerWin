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

	inline float RandomRange(float Start, float End)
	{
		return Start + Random() * (End - Start);
	}

	template<typename T>
	T Clamp(T& Input, const T& MinValue, const T& MaxValue)
	{
		return Input < MinValue ? MinValue : Input > MaxValue ? MaxValue : Input;
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

	// Compute barycentric coordinates (u, v, w) for
	// point p with respect to triangle (a, b, c)
	void Barycentric(const RVec3& p, const RVec3& a, const RVec3& b, const RVec3& c, float& u, float& v, float &w);
}
