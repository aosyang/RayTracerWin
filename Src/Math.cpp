//=============================================================================
// Math.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "Math.h"

namespace RMath
{
    
    RVec3 RandomHemisphereDirection(const RVec3& Normal)
    {
        RVec3 v = RandomUnitSphereDirection();
        
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
