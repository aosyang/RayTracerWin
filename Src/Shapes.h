//=============================================================================
// Shapes.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RVector.h"
#include "RRay.h"

// Base shape class
class RShape
{
public:
	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const { return false; }
    
protected:
    RAabb Aabb;
};


// Sphere
class RSphere : public RShape
{
public:
	RVec3 Center;
	float Radius;

	RSphere(const RVec3& InCenter, float InRadius)
		: Center(InCenter), Radius(InRadius)
	{
        Aabb.ExpandBySphere(Center, Radius);
    }

	static RShape* Create(const RVec3& InCenter, float InRadius) { return new RSphere(InCenter, InRadius); }

	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const override;
};

// Plane
class RPlane : public RShape
{
public:
	RVec3 Normal;
	RVec3 Point;

	RPlane(const RVec3& InNormal, const RVec3& InPoint)
		: Normal(InNormal), Point(InPoint)
	{}

	static RShape* Create(const RVec3& InNormal, const RVec3& InPoint) { return new RPlane(InNormal, InPoint); }

	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const override;
};

// Capsule
class RCapsule : public RShape
{
public:
	RVec3 Start;
	RVec3 End;
	float Radius;

	RCapsule(const RVec3& InStart, const RVec3& InEnd, float InRadius)
		: Start(InStart), End(InEnd), Radius(InRadius)
	{
        Aabb.ExpandBySphere(Start, Radius);
        Aabb.ExpandBySphere(End, Radius);
    }

	static RShape* Create(const RVec3& InStart, const RVec3& InEnd, float InRadius) { return new RCapsule(InStart, InEnd, InRadius); }

	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const override;

protected:
	bool TestRayCylinderIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const;
};
