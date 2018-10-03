//=============================================================================
// RRay.cpp by Shiyang Ao, 2016 All Rights Reserved.
//
// 
//=============================================================================

#include "RRay.h"

RRay::RRay()
{
}

RRay::RRay(const RVec3& _origin, const RVec3& _dir, float _dist)
	: Origin(_origin), Direction(_dir), Distance(_dist)
{
}

RRay::RRay(const RVec3& _start, const RVec3& _end)
	: Origin(_start), Direction((_end - _start).GetNormalizedVec3()), Distance((_end - _start).Magnitude())
{
}

bool RRay::TestIntersectionWithSphere(const RVec3& SphereCenter, float SphereRadius, RayHitResult* result /*= nullptr*/) const
{
	// Reference: http://www.ccs.neu.edu/home/fell/CSU540/programs/RayTracingFormulas.htm

	float dx = Direction.x * Distance;
	float dy = Direction.y * Distance;
	float dz = Direction.z * Distance;

	float _a = dx*dx + dy*dy + dz*dz;
	float _b = 2 * dx*(Origin.x - SphereCenter.x) + 2 * dy*(Origin.y - SphereCenter.y) + 2 * dz*(Origin.z - SphereCenter.z);
	float _c = SphereCenter.x*SphereCenter.x + SphereCenter.y*SphereCenter.y + SphereCenter.z*SphereCenter.z + Origin.x*Origin.x + Origin.y*Origin.y + Origin.z*Origin.z +
		      -2 * (SphereCenter.x*Origin.x + SphereCenter.y*Origin.y + SphereCenter.z*Origin.z) - SphereRadius*SphereRadius;

	float d = _b*_b - 4 * _a*_c;

	if (d >= 0)
	{
		float t = (-_b - sqrtf(d)) / (_a * 2);

		if (t <= 0)
			return false;

		RVec3 hitPoint = RVec3(Origin.x + t * dx, Origin.y + t * dy, Origin.z + t * dz);
		float dist = (hitPoint - Origin).Magnitude();

		if (dist > Distance)
			return false;

		if (result)
		{
			result->HitPosition = hitPoint;
			result->HitNormal = (hitPoint - SphereCenter).GetNormalizedVec3();
			result->Distance = dist;
		}

		return true;
	}

	return false;
}

bool RRay::TestIntersectionWithPlane(const RVec3& PlaneNormal, const RVec3& PointOnPlane, RayHitResult* result /*= nullptr*/) const
{
	// assuming vectors are all normalized
	float denom = PlaneNormal.Dot(Direction);
	if (fabsf(denom) > 1e-6) {
		RVec3 p0l0 = PointOnPlane - Origin;
		float t = p0l0.Dot(PlaneNormal) / denom;
		if (t >= 0 && t < Distance)
		{
			if (result)
			{
				result->HitPosition = Origin + Direction * t;
				result->HitNormal = PlaneNormal;
				result->Distance = t;
			}

			return true;
		}
	}

	return false;
}

bool RRay::TestIntersectionWithAabb(const RAabb& aabb, float* t/*=nullptr*/) const
{
    if (!aabb.IsValid())
        return false;

    float tmin = -FLT_MAX, tmax = FLT_MAX;

    if (!FLT_EQUAL_ZERO(Direction.x))
    {
		float inv_x = 1.0f / Direction.x;
        float tx1 = (aabb.pMin.x - Origin.x) * inv_x;
        float tx2 = (aabb.pMax.x - Origin.x) * inv_x;

        tmin = Math::Max(tmin, Math::Min(tx1, tx2));
        tmax = Math::Min(tmax, Math::Max(tx1, tx2));
    }

    if (!FLT_EQUAL_ZERO(Direction.y))
    {
		float inv_y = 1.0f / Direction.y;
		float ty1 = (aabb.pMin.y - Origin.y) * inv_y;
        float ty2 = (aabb.pMax.y - Origin.y) * inv_y;

        tmin = Math::Max(tmin, Math::Min(ty1, ty2));
        tmax = Math::Min(tmax, Math::Max(ty1, ty2));
    }

    if (!FLT_EQUAL_ZERO(Direction.z))
    {
		float inv_z = 1.0f / Direction.z;
        float tz1 = (aabb.pMin.z - Origin.z) * inv_z;
        float tz2 = (aabb.pMax.z - Origin.z) * inv_z;

        tmin = Math::Max(tmin, Math::Min(tz1, tz2));
        tmax = Math::Min(tmax, Math::Max(tz1, tz2));
    }

    if (tmax > tmin)
    {
		if (t)
		{
			*t = tmin;
		}

        return true;
    }

    return false;
}

