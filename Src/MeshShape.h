//=============================================================================
// MeshShape.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Shapes.h"
#include "KdTree.h"

#include <vector>

class RMeshShape : public RShape
{
public:
	RMeshShape(const char* Filename);

	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const override;

	static unique_ptr<RMeshShape> Create(const char* Filename) { return std::make_unique<RMeshShape>(Filename); }

private:
	std::vector<RVec3>		Points;
	std::vector<RVec3>		FaceNormals;
	std::vector<int>		Indices;

	unique_ptr<KdTree>		Spatial;
};
