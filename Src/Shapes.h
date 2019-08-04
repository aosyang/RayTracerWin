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
    virtual ~RShape() {}
    
	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const { return false; }

	// Assign material to the shape
	void SetMaterial(RMaterial Material);

	// Get material of the shape
	const RMaterial& GetMaterial() const;

	// Whether to run aabb culling on this shape
	virtual bool HasCullingBounds() const { return true; }

	// Get the bounds of this shape for culling
	const RAabb& GetBounds() const;
    
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

	static unique_ptr<RSphere> Create(const RVec3& InCenter, float InRadius) { return std::unique_ptr<RSphere>(new RSphere(InCenter, InRadius)); }

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

	static unique_ptr<RShape> Create(const RVec3& InNormal, const RVec3& InPoint) { return std::unique_ptr<RPlane>(new RPlane(InNormal, InPoint)); }

	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const override;

	virtual bool HasCullingBounds() const override;
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

	static unique_ptr<RShape> Create(const RVec3& InStart, const RVec3& InEnd, float InRadius) { return std::unique_ptr<RCapsule>(new RCapsule(InStart, InEnd, InRadius)); }

	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const override;

protected:
	bool TestRayCylinderIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const;
};

class RTriangle : public RShape
{
public:
	RVec3 Points[3];

	RTriangle(const RVec3& p0, const RVec3& p1, const RVec3& p2)
	{
		Points[0] = p0;
		Points[1] = p1;
		Points[2] = p2;

		Aabb.Expand(p0);
		Aabb.Expand(p1);
		Aabb.Expand(p2);
	}

	static unique_ptr<RShape> Create(const RVec3& p0, const RVec3& p1, const RVec3& p2) { return std::unique_ptr<RTriangle>(new RTriangle(p0, p1, p2)); }

	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult /* = nullptr */) const override;
};
