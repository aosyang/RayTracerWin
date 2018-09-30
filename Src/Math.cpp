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
    
}
