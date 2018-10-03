//=============================================================================
// Shapes.h by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#pragma once

#include "RVector.h"
#include "RRay.h"
#include "Material.h"

#include <memory>

using std::unique_ptr;

// Base shape class
class RShape
{
public:
	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const { return false; }

	// Assign material to the shape
	void SetMaterial(RMaterial Material);

	// Get material of the shape
	const RMaterial& GetMaterial() const;
    
protected:
	// World bounding box of the shape
    RAabb Aabb;

	// Material used by the shape
	RMaterial ShapeMaterial;
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

	static unique_ptr<RSphere> Create(const RVec3& InCenter, float InRadius) { return std::make_unique<RSphere>(InCenter, InRadius); }

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

	static unique_ptr<RShape> Create(const RVec3& InNormal, const RVec3& InPoint) { return std::make_unique<RPlane>(InNormal, InPoint); }

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

	static unique_ptr<RShape> Create(const RVec3& InStart, const RVec3& InEnd, float InRadius) { return std::make_unique<RCapsule>(InStart, InEnd, InRadius); }

	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const override;

protected:
	bool TestRayCylinderIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const;
};
