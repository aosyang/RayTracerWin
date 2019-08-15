//=============================================================================
// MathHelper.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "MathHelper.h"
#include "Platform.h"

#include <stdlib.h>

namespace Math
{
	// Returns random float in [0, 1].
	float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b].
	float RandF(float a, float b)
	{
		return a + RandF()*(b - a);
	}
    
    float Q_rsqrt(float number)
    {
        const float x2 = number * 0.5F;
        const float threehalfs = 1.5F;
        
        union {
            float f;
            UINT32 i;
        } conv = {number}; // member 'f' set to value of 'number'.
        conv.i  = 0x5f3759df - ( conv.i >> 1 );
        conv.f  *= ( threehalfs - ( x2 * conv.f * conv.f ) );
        return conv.f;
    }
}
