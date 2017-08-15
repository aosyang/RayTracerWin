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
	: Origin(_origin), Direction(_dir.GetNormalizedVec3()), Distance(_dist)
{
}

RRay::RRay(const RVec3& _start, const RVec3& _end)
	: Origin(_start), Direction((_end - _start).GetNormalizedVec3()), Distance((_end - _start).Magnitude())
{
}

bool RRay::TestSphereIntersection(const RVec3& c, float r, RayHitResult* result) const
{
	// Reference: http://www.ccs.neu.edu/home/fell/CSU540/programs/RayTracingFormulas.htm

	float dx = Direction.x * Distance;
	float dy = Direction.y * Distance;
	float dz = Direction.z * Distance;

	float _a = dx*dx + dy*dy + dz*dz;
	float _b = 2 * dx*(Origin.x - c.x) + 2 * dy*(Origin.y - c.y) + 2 * dz*(Origin.z - c.z);
	float _c = c.x*c.x + c.y*c.y + c.z*c.z + Origin.x*Origin.x + Origin.y*Origin.y + Origin.z*Origin.z +
		      -2 * (c.x*Origin.x + c.y*Origin.y + c.z*Origin.z) - r*r;

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
			result->hitPoint = hitPoint;
			result->hitNormal = (hitPoint - c).GetNormalizedVec3();
			result->dist = dist;
		}

		return true;
	}

	return false;
}

bool RRay::TestPlaneIntersection(const RVec3& n, const RVec3& p0, RayHitResult* result /*= nullptr*/) const
{
	// assuming vectors are all normalized
	float denom = n.Dot(Direction);
	if (fabsf(denom) > 1e-6) {
		RVec3 p0l0 = p0 - Origin;
		float t = p0l0.Dot(n) / denom;
		if (t >= 0 && t < Distance)
		{
			if (result)
			{
				result->hitPoint = Origin + Direction * t;
				result->hitNormal = n;
				result->dist = t;
			}

			return true;
		}
	}

	return false;
}

//RRay RRay::Transform(const RMatrix4& mat) const
//{
//	return RRay((RVec4(Origin, 1.0f) * mat).ToVec3(), (RVec4(Direction, 0.0f) * mat).ToVec3(), Distance);
//}
//
//bool RRay::TestAabbIntersection(const RAabb& aabb, float* t/*=nullptr*/) const
//{
//	if (!aabb.IsValid())
//		return false;
//
//	float tmin = -FLT_MAX, tmax = FLT_MAX;
//
//	if (!FLT_EQUAL_ZERO(Direction.x))
//	{
//		float tx1 = (aabb.pMin.x - Origin.x) / Direction.x;
//		float tx2 = (aabb.pMax.x - Origin.x) / Direction.x;
//
//		tmin = max(tmin, min(tx1, tx2));
//		tmax = min(tmax, max(tx1, tx2));
//	}
//
//	if (!FLT_EQUAL_ZERO(Direction.y))
//	{
//		float ty1 = (aabb.pMin.y - Origin.y) / Direction.y;
//		float ty2 = (aabb.pMax.y - Origin.y) / Direction.y;
//
//		tmin = max(tmin, min(ty1, ty2));
//		tmax = min(tmax, max(ty1, ty2));
//	}
//
//	if (!FLT_EQUAL_ZERO(Direction.z))
//	{
//		float tz1 = (aabb.pMin.z - Origin.z) / Direction.z;
//		float tz2 = (aabb.pMax.z - Origin.z) / Direction.z;
//
//		tmin = max(tmin, min(tz1, tz2));
//		tmax = min(tmax, max(tz1, tz2));
//	}
//
//	if (tmax > tmin)
//	{
//		if (t)
//			*t = tmin;
//		return true;
//	}
//
//	return false;
//}
