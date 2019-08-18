//=============================================================================
// MeshShape.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Shapes.h"
#include "KdTree.h"
#include "Texture.h"

#include <vector>

class RMeshShape : public RShape
{
public:
	RMeshShape(const std::string& Filename);

	virtual bool TestRayIntersection(const RRay& InRay, RayHitResult* OutResult = nullptr) const override;

	static unique_ptr<RMeshShape> Create(const std::string& Filename) { return std::unique_ptr<RMeshShape>(new RMeshShape(Filename)); }

private:
	std::vector<RVec3>		Points;
	std::vector<RVec3>		Texcoords;
	std::vector<RVec3>		Normals;
	std::vector<RVec3>		FaceNormals;
	std::vector<int>		PointIndices;
	std::vector<int>		TexcoordIndices;
	std::vector<int>		NormalIndices;

	// Per-polygon material id
	std::vector<int>		PolyMaterialId;
	std::vector<std::unique_ptr<RTexture>>	Textures;

	unique_ptr<KdTree>		Spatial;
};
