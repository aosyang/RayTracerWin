//=============================================================================
// KdTree.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "KdTree.h"
#include "RAabb.h"

EAxis GetLargestAxisOfBounds(const RAabb& Bounds)
{
	RVec3 size = Bounds.pMax - Bounds.pMin;
	if (size.x > size.y)
	{
		if (size.x > size.z)
		{
			return EAxis::X;
		}
		else
		{
			return EAxis::Z;
		}
	}
	else	// size.y >= size.x
	{
		if (size.y > size.z)
		{
			return EAxis::Y;
		}
		else
		{
			return EAxis::Z;
		}
	}
}

void KdNode::Build(const RVec3 Points[], const std::vector<TriangleIndex>& Triangles)
{
	int NumTriangles = (int)Triangles.size();

	// Measure bounds for all points
	for (int i = 0; i < NumTriangles; i++)
	{
		Bounds.Expand(Points[Triangles[i].p0]);
		Bounds.Expand(Points[Triangles[i].p1]);
		Bounds.Expand(Points[Triangles[i].p2]);
	}

	// Only one triangle in the list, make current node a leaf node
	if (Triangles.size() == 1)
	{
		Triangle = Triangles[0];

		return;
	}

	RVec3 NodeMidPoint(0, 0, 0);
	for (int i = 0; i < NumTriangles; i++)
	{
		const RVec3& v0 = Points[Triangles[i].p0];
		const RVec3& v1 = Points[Triangles[i].p1];
		const RVec3& v2 = Points[Triangles[i].p2];

		NodeMidPoint += (v0 + v1 + v2) / 3.0f;
	}
	NodeMidPoint /= (float)NumTriangles;

	std::vector<TriangleIndex> LeftNodeTriangles;
	std::vector<TriangleIndex> RightNodeTriangles;
	const EAxis Axis = GetLargestAxisOfBounds(Bounds);

	for (int i = 0; i < NumTriangles; i++)
	{
		const RVec3& v0 = Points[Triangles[i].p0];
		const RVec3& v1 = Points[Triangles[i].p1];
		const RVec3& v2 = Points[Triangles[i].p2];

		RVec3 MidPoint = (v0 + v1 + v2) / 3.0f;

		bool bInsertToLeft = false;

		switch (Axis)
		{
		case EAxis::X:
			bInsertToLeft = (MidPoint.x < NodeMidPoint.x);
			break;

		case EAxis::Y:
			bInsertToLeft = (MidPoint.y < NodeMidPoint.y);
			break;

		case EAxis::Z:
			bInsertToLeft = (MidPoint.z < NodeMidPoint.z);
			break;
		}

		if (bInsertToLeft)
		{
			LeftNodeTriangles.emplace_back(Triangles[i]);
		}
		else
		{
			RightNodeTriangles.emplace_back(Triangles[i]);
		}
	}

	// All triangles go into one side of child node? Let's split them half-half for both nodes.
	if (LeftNodeTriangles.size() == Triangles.size() || RightNodeTriangles.size() == Triangles.size())
	{
		const size_t half_size = Triangles.size() / 2;
		LeftNodeTriangles = std::vector<TriangleIndex>(Triangles.begin(), Triangles.begin() + half_size);
		RightNodeTriangles = std::vector<TriangleIndex>(Triangles.begin() + half_size, Triangles.end());
	}

	if (LeftNodeTriangles.size() > 0)
	{
		Left = std::make_unique<KdNode>();
		Left->Build(Points, LeftNodeTriangles);
	}

	if (RightNodeTriangles.size() > 0)
	{
		Right = std::make_unique<KdNode>();
		Right->Build(Points, RightNodeTriangles);
	}
}

bool KdNode::TestRayIntersection(RRay& TestRay, const RVec3 Points[], RayHitResult* OutResult /*= nullptr*/) const
{
	if (!TestRay.TestIntersectionWithAabb(Bounds))
	{
		return false;
	}

	bool bIsLeaf = true;
	bool bResult = false;

	if (Left)
	{
		bResult |= Left->TestRayIntersection(TestRay, Points, OutResult);
		bIsLeaf = false;
	}

	if (Right)
	{
		bResult |= Right->TestRayIntersection(TestRay, Points, OutResult);
		bIsLeaf = false;
	}

	if (bIsLeaf)
	{
		const RVec3 TriPoints[] = {
			Points[Triangle.p0],
			Points[Triangle.p1],
			Points[Triangle.p2],
		};
		
		RayHitResult HitResult;
		if (TestRay.TestIntersectionWithTriangle(TriPoints, &HitResult))
		{
			TestRay.Distance = HitResult.Distance;
				
			if (OutResult)
			{
				*OutResult = HitResult;
			}

			return true;
		}

		return false;
	}

	return bResult;
}

KdTree::KdTree()
{

}

void KdTree::Build(const RVec3 Points[], const int Indices[], int NumIndices)
{
	const TriangleIndex* Triangles = reinterpret_cast<const TriangleIndex*>(Indices);
	const int NumTriangles = NumIndices / 3;

	std::vector<TriangleIndex> TriangleIndices(Triangles, Triangles + NumTriangles);
	RootNode = std::make_unique<KdNode>();

	RootNode->Build(Points, TriangleIndices);
}

bool KdTree::TestRayIntersection(const RRay& InRay, const RVec3 Points[], RayHitResult* OutResult /*= nullptr*/) const
{
	if (RootNode == nullptr)
	{
		return false;
	}

	RRay TestRay = InRay;

	return RootNode->TestRayIntersection(TestRay, Points, OutResult);
}

RAabb KdTree::GetBounds() const
{
	if (RootNode)
	{
		return RootNode->Bounds;
	}

	static const RAabb InvalidBounds = RAabb();
	return InvalidBounds;
}
