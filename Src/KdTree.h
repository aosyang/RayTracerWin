//=============================================================================
// KdTree.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RVector.h"
#include "RAabb.h"

#include <memory>
#include <vector>
#include "RRay.h"

using std::unique_ptr;

enum class EAxis : unsigned char
{
	X,
	Y,
	Z,
};

struct TriangleData
{
	TriangleData()
		: p0(-1)
		, p1(-1)
		, p2(-1)
		, Index(-1)
	{}

	TriangleData(int InP0, int InP1, int InP2, int InIndex)
		: p0(InP0)
		, p1(InP1)
		, p2(InP2)
		, Index(InIndex)
	{}

	TriangleData(const TriangleData& rhs)
		: p0(rhs.p0)
		, p1(rhs.p1)
		, p2(rhs.p2)
		, Index(rhs.Index)
	{}

	RAabb GetBounds(const RVec3 Points[]) const
	{
		RAabb Bounds;
		Bounds.Expand(Points[p0]);
		Bounds.Expand(Points[p1]);
		Bounds.Expand(Points[p2]);

		return Bounds;
	}

	int p0;
	int p1;
	int p2;
	int Index;		// Index of triangle in original mesh
};

struct KdNode
{
	unique_ptr<KdNode> Left;
	unique_ptr<KdNode> Right;

	TriangleData Triangle;
	RAabb Bounds;

	KdNode() {}

	void Build(const RVec3 Points[], const std::vector<TriangleData>& Triangles);

	bool TestRayIntersection(RRay& TestRay, const RVec3 Points[], RayHitResult* OutResult = nullptr, int* TriangleIndex = nullptr) const;
};

class KdTree
{
public:
	KdTree();

	// Construct a tree from triangle list
	void Build(const RVec3 Points[], const int Indices[], int NumIndices);

	// Test intersection with ray
	bool TestRayIntersection(const RRay& InRay, const RVec3 Points[], RayHitResult* OutResult = nullptr, int* TriangleIndex = nullptr) const;

	// Get the bounds of this kd-tree
	RAabb GetBounds() const;

private:
	unique_ptr<KdNode> RootNode;
};
