//=============================================================================
// MeshShape.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "MeshShape.h"
#include "Math.h"
#include "Platform.h"

#include <fstream>
#include <string>
#include <sstream>

static std::string GetLineKeyword(const std::string& Line)
{
	std::string Keyword;

	size_t SpacePos = Line.find(' ');
	if (SpacePos != std::string::npos)
	{
		return Line.substr(0, SpacePos);
	}

	return Line;
}

static int GetNthNumericValue(int n, const std::string& Line)
{
	std::stringstream LineStream(Line);
	int value;
	char dummy;
	for (int i = 0; i <= n; i++)
	{
		LineStream >> value >> dummy;
	}
	return value;
}

RMeshShape::RMeshShape(const char* Filename)
{
	std::ifstream InputFile(Filename);
	
	if (InputFile.is_open())
	{
		std::string Line;
		while (std::getline(InputFile, Line))
		{
			std::string key = GetLineKeyword(Line);
			std::string dummy;

			if (key == "v")
			{
				std::stringstream LineStream(Line);
				float Pos[3];
				LineStream >> dummy >> Pos[0] >> Pos[1] >> Pos[2];
				
				// Hack: Move mesh away from camera
				Pos[2] += 1.0f;

				RVec3 Vertex(Pos);
				Points.push_back(Vertex);
				Aabb.Expand(Vertex);
			}
			else if (key == "vn")
			{
				std::stringstream LineStream(Line);
				float n[3];
				LineStream >> dummy >> n[0] >> n[1] >> n[2];
				Normals.push_back(RVec3(n));
			}
			else if (key == "f")
			{
				std::stringstream LineStream(Line);
				std::string IndexString[3];
				LineStream >> dummy >> IndexString[0] >> IndexString[1] >> IndexString[2];

				for (int i = 0; i < 3; i++)
				{
					int PointIndex = GetNthNumericValue(0, IndexString[i]) - 1;
					PointIndices.push_back(PointIndex);

					int NormalIndex = GetNthNumericValue(2, IndexString[i]) - 1;
					NormalIndices.push_back(NormalIndex);
				}
			}
		}

		InputFile.close();
		RLog("Mesh loaded from %s. Verts: %d, Triangles: %d\n", Filename, (int)Points.size(), (int)PointIndices.size() / 3);

		for (int i = 0; i < (int)PointIndices.size(); i += 3)
		{
			const RVec3& p0 = Points[PointIndices[i]];
			const RVec3& p1 = Points[PointIndices[i + 1]];
			const RVec3& p2 = Points[PointIndices[i + 2]];

			RVec3 p0p1 = p1 - p0;
			RVec3 p0p2 = p2 - p0;
			RVec3 Normal = p0p1.Cross(p0p2).GetNormalizedVec3();

			FaceNormals.push_back(Normal);
		}

		Spatial = std::unique_ptr<KdTree>(new KdTree());
		Spatial->Build(Points.data(), PointIndices.data(), (int)PointIndices.size());
	}
	else
	{
		RLog("Error - RMeshShape: Unable to open %s!\n", Filename);
	}
}

bool RMeshShape::TestRayIntersection(const RRay& InRay, RayHitResult* OutResult /*= nullptr*/) const
{
#if 1
	if (Spatial)
	{
		int TriangleIndex = -1;
		if (Spatial->TestRayIntersection(InRay, Points.data(), OutResult, &TriangleIndex))
		{
			if (OutResult)
			{
				const RVec3& p = OutResult->HitPosition;

				int v0 = TriangleIndex * 3;
				int v1 = TriangleIndex * 3 + 1;
				int v2 = TriangleIndex * 3 + 2;

				const RVec3& a = Points[PointIndices[v0]];
				const RVec3& b = Points[PointIndices[v1]];
				const RVec3& c = Points[PointIndices[v2]];

				const RVec3& n0 = Normals[NormalIndices[v0]];
				const RVec3& n1 = Normals[NormalIndices[v1]];
				const RVec3& n2 = Normals[NormalIndices[v2]];

				float u, v, w;
				RMath::Barycentric(p, a, b, c, u, v, w);

				OutResult->HitNormal = n0 * u + n1 * v + n2 * w;
			}

			return true;
		}
	}
	return false;
#else
	RRay TestRay = InRay;
	bool bResult = false;

	for (int i = 0; i < (int)PointIndices.size(); i += 3)
	{
		const RVec3 TriPoints[] = {
			Points[PointIndices[i]],
			Points[PointIndices[i + 1]],
			Points[PointIndices[i + 2]],
		};

		const RVec3& Normal = FaceNormals[i / 3];
		
		if (TestRay.TestIntersectionWithTriangleAndFaceNormal(TriPoints, Normal, OutResult))
		{
			TestRay.Distance = OutResult->Distance;

			bResult = true;
		}
	}

	return bResult;
#endif
}
