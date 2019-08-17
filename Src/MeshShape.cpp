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

using namespace std;

#define USE_KDTREE 1

namespace
{
    // Get the identifier string of a line (string before the first space)
    string GetLineKeyword(const string& Line)
    {
        string Keyword;
        
        size_t SpacePos = Line.find(' ');
        if (SpacePos != string::npos)
        {
            return Line.substr(0, SpacePos);
        }
        
        return Line;
    }
    
    // Get the nth number from a line
    int GetNthNumericValue(int n, const string& Line)
    {
        stringstream LineStream(Line);
        int value = -1;
        char dummy;
        for (int i = 0; i <= n; i++)
        {
            LineStream >> value >> dummy;
        }
        return value;
    }
    
    // Split a string by a given delimiter
    vector<string> Split(const string& Input, char Delimiter)
    {
        vector<string> tokens;
        string token;
        istringstream tokenStream(Input);
        while (getline(tokenStream, token, Delimiter))
        {
            tokens.push_back(token);
        }
        
        return tokens;
    }
}

RMeshShape::RMeshShape(const char* Filename)
{
	ifstream InputFile(Filename);
	
	if (!InputFile.is_open())
	{
		// If file is not found, search an alternative path for it.
		char AlterPath[1024];
		RPrintf(AlterPath, sizeof(AlterPath), "../%s", Filename);
		InputFile = ifstream(AlterPath);

		if (!InputFile.is_open())
		{
			RLog("Error - RMeshShape: Unable to open %s!\n", Filename);
			return;
		}
	}

	string Line;
	while (getline(InputFile, Line))
	{
		string key = GetLineKeyword(Line);
		string dummy;

		if (key == "v")
		{
			stringstream LineStream(Line);
			float Pos[3];
			LineStream >> dummy >> Pos[0] >> Pos[1] >> Pos[2];

			RVec3 Vertex(Pos);
			Points.push_back(Vertex);
			Aabb.Expand(Vertex);
		}
		else if (key == "vn")
		{
			stringstream LineStream(Line);
			float n[3];
			LineStream >> dummy >> n[0] >> n[1] >> n[2];
			Normals.push_back(RVec3(n));
		}
		else if (key == "f")
		{
            // Note: the first token will be the key
            auto Tokens = Split(Line, ' ');
            int NumPolyVerts = (int)Tokens.size() - 1;
            const vector<int>* PolyIndexArray = nullptr;
            
            if (NumPolyVerts == 3)      // A triangle poly
            {
                static const vector<int> TriangleIndex{ 0, 1, 2 };
                PolyIndexArray = &TriangleIndex;
            }
            else if (NumPolyVerts == 4) // A quad poly
            {
                // Split a quad into two triangle
                static const vector<int> QuadIndex{ 0, 1, 2, 0, 2, 3 };
                PolyIndexArray = &QuadIndex;
            }
            
            if (PolyIndexArray != nullptr)
            {
                for (int i = 0; i < (int)PolyIndexArray->size(); i++)
                {
                    int Index = (*PolyIndexArray)[i] + 1;
                    
                    int PointIndex = GetNthNumericValue(0, Tokens[Index]) - 1;
                    PointIndices.push_back(PointIndex);
                    
                    int NormalIndex = GetNthNumericValue(2, Tokens[Index]) - 1;
                    NormalIndices.push_back(NormalIndex);
                }
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
		RVec3 Normal = RVec3::Cross(p0p1, p0p2).GetNormalizedVec3();

		FaceNormals.push_back(Normal);
	}

	Spatial = unique_ptr<KdTree>(new KdTree());
	Spatial->Build(Points.data(), PointIndices.data(), (int)PointIndices.size());
}

bool RMeshShape::TestRayIntersection(const RRay& InRay, RayHitResult* OutResult /*= nullptr*/) const
{
#if USE_KDTREE
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

                // Use fast inverse square root for approximating normal direction
				OutResult->HitNormal = (n0 * u + n1 * v + n2 * w).GetNormalizedVec3_Fast();
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
#endif  // if USE_KDTREE
}
