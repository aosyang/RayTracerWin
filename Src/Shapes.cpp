//=============================================================================
// Shapes.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "Shapes.h"

bool RSphere::TestRayIntersection(const RRay& InRay, RayHitResult* OutResult /*= nullptr*/) const
{
	return InRay.TestIntersectionWithSphere(Center, Radius, OutResult);
}

bool RPlane::TestRayIntersection(const RRay& InRay, RayHitResult* OutResult /*= nullptr*/) const
{
	return InRay.TestIntersectionWithPlane(Normal, Point, OutResult);
}

bool RCapsule::TestRayIntersection(const RRay& InRay, RayHitResult* OutResult /*= nullptr*/) const
{
	if (!TestRayCylinderIntersection(InRay, OutResult))
	{
		RayHitResult r1, r2;

		bool b1 = InRay.TestIntersectionWithSphere(Start, Radius, &r1);
		bool b2 = InRay.TestIntersectionWithSphere(End, Radius, &r2);

		if (OutResult)
		{
			if (b1 && b2)
			{
				*OutResult = r1.Distance < r2.Distance ? r1 : r2;
			}
			else if (b1)
			{
				*OutResult = r1;
			}
			else if (b2)
			{
				*OutResult = r2;
			}
		}

		return b1 || b2;
	}

	return true;
}

bool RCapsule::TestRayCylinderIntersection(const RRay& InRay, RayHitResult* OutResult /*= nullptr*/) const
{
    RVec3 d = End - Start; // vector from first point on cylinder segment to the end point on cylinder segment
	RVec3 m = InRay.Origin - Start; // vector from first point on cylinder segment to start point of ray

	// Values used to calculate coefficients of quadratic formula.
	// You do not necessarily have to use any of these directly for the rest of the algorithm.
	float dd = RVec3::Dot( d, d ); // dot product of d with d (squared magnitude of d)
	float nd = RVec3::Dot( InRay.Direction, d ); // dot product of ray normal (n) with d
	float mn = RVec3::Dot( m, InRay.Direction );
	float md = RVec3::Dot( m, d ); 
	float mm = RVec3::Dot( m, m ); 


	// TODO: Optimization by early out
	//		 If the ray starts outside the top or bottom planes and points away, there can be no intersection.
	if (RVec3::Dot(InRay.Origin - Start, End - Start) < 0 && RVec3::Dot(InRay.Direction, End - Start) < 0)
		return false;

	if (RVec3::Dot(InRay.Origin - End, Start - End) < 0 && RVec3::Dot(InRay.Direction, Start - End) < 0)
		return false;

	// Coefficients for the quadratic formula
	float a = dd - nd * nd;
	float b = dd*mn - nd*md;
	float c = dd*(mm - Radius*Radius) - md*md;

	// If a is approximately 0.0 then the ray is parallel to the cylinder and can't intersect
	if( fabs(a) < FLT_EPSILON )
		return false;

	// TODO: Find time of intersection, if any
	//		 Use the quadratic formula to solve for t. Reference "Advanced Ray to Sphere.ppt" for an example.
	//		 As with "Advanced Ray to Sphere", the 2s and 4 in the formula ( x = (-b - sqrt(b*b - 4ac)) / 2a )
	//		 are cancelled out, resulting in a simplified form.
	if ((b * b - a * c) < 0)
		return false;

	float r_t = (-b - sqrtf(b * b - a * c)) / a;

	if (r_t < 0)
		return false;

	RVec3 v = InRay.Origin + InRay.Direction * r_t;
	if (RVec3::Dot(v - Start, End - Start) < 0)
		return false;

	if (RVec3::Dot(v - End, Start - End) < 0)
		return false;

	if (OutResult)
	{
		OutResult->Distance = r_t;
		OutResult->HitPosition = InRay.Origin + InRay.Direction * r_t;

		RVec3 SideVec = (End - Start).Cross(OutResult->HitPosition - Start);
		OutResult->HitNormal = SideVec.Cross(End - Start).GetNormalizedVec3();
	}

	return true;
}
