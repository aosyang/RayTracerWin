//=============================================================================
// RRay.h by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================
#ifndef _RRAY_H
#define _RRAY_H

#include "RVector.h"
#include "RAabb.h"

// Ray hitting information
struct RayHitResult
{
	RVec3 HitPosition;
	RVec3 HitNormal;
	float Distance;
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

	bool TestIntersectionWithSphere(const RVec3& SphereCenter, float SphereRadius, RayHitResult* result = nullptr) const;
	bool TestIntersectionWithPlane(const RVec3& PlaneNormal, const RVec3& PointOnPlane, RayHitResult* result = nullptr) const;

	bool TestIntersectionWithAabb(const RAabb& aabb, float* t = nullptr) const;
};

#endif
