//=============================================================================
// MeshShape.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "MeshShape.h"

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

static int GetFirstNumericValue(const std::string& Line)
{
	std::stringstream LineStream(Line);
	int n;
	LineStream >> n;
	return n;
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
			else if (key == "f")
			{
				std::stringstream LineStream(Line);
				std::string IndexString[3];
				LineStream >> dummy >> IndexString[0] >> IndexString[1] >> IndexString[2];

				for (int i = 0; i < 3; i++)
				{
					int Index = GetFirstNumericValue(IndexString[i]) - 1;
					Indices.push_back(Index);
				}
			}
		}

		InputFile.close();
	}

	for (int i = 0; i < (int)Indices.size(); i += 3)
	{
		const RVec3& p0 = Points[Indices[i]];
		const RVec3& p1 = Points[Indices[i + 1]];
		const RVec3& p2 = Points[Indices[i + 2]];

		RVec3 p0p1 = p1 - p0;
		RVec3 p0p2 = p2 - p0;
		RVec3 Normal = p0p1.Cross(p0p2).GetNormalizedVec3();

		FaceNormals.push_back(Normal);
	}
}

bool RMeshShape::TestRayIntersection(const RRay& InRay, RayHitResult* OutResult /*= nullptr*/) const
{
	RRay TestRay = InRay;
	bool bResult = false;

	for (int i = 0; i < (int)Indices.size(); i += 3)
	{
		const RVec3 TriPoints[] = {
			Points[Indices[i]],
			Points[Indices[i + 1]],
			Points[Indices[i + 2]],
		};

		const RVec3& Normal = FaceNormals[i / 3];
		
		if (TestRay.TestIntersectionWithTriangleAndFaceNormal(TriPoints, Normal, OutResult))
		{
			TestRay.Distance = OutResult->Distance;

			bResult = true;
		}
	}

	return bResult;
}
