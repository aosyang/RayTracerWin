//=============================================================================
// RAabb.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
//
//=============================================================================

#include "RAabb.h"

#include <float.h>

RAabb RAabb::Default;

RAabb::RAabb()
:pMin(FLT_MAX, FLT_MAX, FLT_MAX),
pMax(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}

bool RAabb::IsValid() const
{
    return pMax.x >= pMin.x && pMax.y >= pMin.y && pMax.z >= pMin.z;
}

bool RAabb::TestPointInsideAabb(const RVec3& point) const
{
    if (pMax.x <= point.x || pMin.x >= point.x)
        return false;
    if (pMax.y <= point.y || pMin.y >= point.y)
        return false;
    if (pMax.z <= point.z || pMin.z >= point.z)
        return false;
    
    return true;
}

bool RAabb::TestIntersectionWithAabb(const RAabb& aabb) const
{
    if (pMax.x <= aabb.pMin.x || pMin.x >= aabb.pMax.x)
        return false;
    if (pMax.y <= aabb.pMin.y || pMin.y >= aabb.pMax.y)
        return false;
    if (pMax.z <= aabb.pMin.z || pMin.z >= aabb.pMax.z)
        return false;
    
    return true;
}

