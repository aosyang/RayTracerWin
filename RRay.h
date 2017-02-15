//=============================================================================
// RRay.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RRAY_H
#define _RRAY_H

#include "RVector.h"
//#include "RAabb.h"

struct RayHitResult
{
	RVec3 hitPoint;
	RVec3 hitNormal;
	float dist;
};

class RRay
{
public:
	RVec3 Origin;
	RVec3 Direction;
	float Distance;

	RRay();
	RRay(const RVec3& _origin, const RVec3& _dir, float _dist);
	RRay(const RVec3& _start, const RVec3& _end);

	bool TestSphereIntersection(const RVec3& c, float r, RayHitResult* result = nullptr) const;
	bool TestPlaneIntersection(const RVec3& n, const RVec3& p0, RayHitResult* result = nullptr) const;

	//RRay Transform(const RMatrix4& mat) const;
	//bool TestAabbIntersection(const RAabb& aabb, float* t = nullptr) const;
};

#endif
