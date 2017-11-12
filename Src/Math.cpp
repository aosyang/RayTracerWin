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
	const char* RandomDataFileName = "pruv.bin";
	const unsigned int MaxUnitVectorNum = 0xFFFFFF;

	RVec3 PseudoRandomUnitVectors[MaxUnitVectorNum];
	unsigned int PseudoRandomIndex = 0;
	std::mutex PseudoRandomUnitVectorMutex;
}

namespace RMath
{
	void InitPseudoRandomUnitVector()
	{
		for (int i = 0; i < MaxUnitVectorNum; i++)
		{
			PseudoRandomUnitVectors[i] = RandomUnitVector();
		}

		PseudoRandomIndex = Math::RandRangedInt(0, MaxUnitVectorNum);
	}
    
	RVec3 PseudoRandomUnitVector()
	{
		std::unique_lock<std::mutex> UniqueLock(PseudoRandomUnitVectorMutex);
		PseudoRandomIndex = PseudoRandomIndex % MaxUnitVectorNum;
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
