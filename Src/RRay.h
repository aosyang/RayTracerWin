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
	RayHitResult()
		: Distance(0.0f)
		, SampledColor(1.0f, 1.0f, 1.0f)
		, SampledAlpha(1.0f)
	{
	}

	RVec3 HitPosition;
	RVec3 HitNormal;
	float Distance;

	// Color from texture
	RVec3 SampledColor;
	float SampledAlpha;
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

	// Ray-sphere intersection test
	bool TestIntersectionWithSphere(const RVec3& SphereCenter, float SphereRadius, RayHitResult* result = nullptr) const;

	// Ray-plane intersection test
	bool TestIntersectionWithPlane(const RVec3& PlaneNormal, const RVec3& PointOnPlane, RayHitResult* result = nullptr) const;

	// Ray-aabb intersection test
	bool TestIntersectionWithAabb(const RAabb& aabb, float* t = nullptr) const;

	// Ray-triangle intersection test
	bool TestIntersectionWithTriangle(const RVec3 TriPoints[3], RayHitResult* result = nullptr) const;

	// Ray-triangle intersection test
	bool TestIntersectionWithTriangleAndFaceNormal(const RVec3 TriPoints[3], const RVec3& Normal, RayHitResult* result = nullptr) const;
};

#endif
