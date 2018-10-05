//=============================================================================
// Math.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================

#include "Math.h"

#include <fstream>
#include <assert.h>
#include <mutex>

namespace
{
	const unsigned int MaxUnitVectorNums = 0xFFFFFF;

	RVec3 PseudoRandomUnitVectors[MaxUnitVectorNums];
}

namespace RMath
{
	void InitPseudoRandomUnitVector()
	{
		// Generate pseudo random unit vectors
		for (int i = 0; i < MaxUnitVectorNums; i++)
		{
			PseudoRandomUnitVectors[i] = RandomUnitVector();
		}
	}
    
	RVec3 PseudoRandomUnitVector()
	{
		// Initialize per-thread pseudo random index
		static unsigned int PseudoRandomIndex = Math::RandRangedInt(0, MaxUnitVectorNums);

		PseudoRandomIndex = PseudoRandomIndex % MaxUnitVectorNums;
		return PseudoRandomUnitVectors[PseudoRandomIndex++];
	}

	RVec3 RandomHemisphereDirection(const RVec3& Normal)
    {
        RVec3 v = PseudoRandomUnitVector();
        
        if (v.Dot(Normal) > 0.0f)
        {
            return v;
        }
        else
        {
            return v.Reflect(Normal);
        }
    }
    
	void Barycentric(const RVec3& p, const RVec3& a, const RVec3& b, const RVec3& c, float& u, float& v, float &w)
	{
		RVec3 v0 = b - a, v1 = c - a, v2 = p - a;
		float d00 = RVec3::Dot(v0, v0);
		float d01 = RVec3::Dot(v0, v1);
		float d11 = RVec3::Dot(v1, v1);
		float d20 = RVec3::Dot(v2, v0);
		float d21 = RVec3::Dot(v2, v1);
		float denom = d00 * d11 - d01 * d01;
		v = (d11 * d20 - d01 * d21) / denom;
		w = (d00 * d21 - d01 * d20) / denom;
		u = 1.0f - v - w;
	}

}
