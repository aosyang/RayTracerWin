//=============================================================================
// MeshShape.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "MeshShape.h"
#include "Math.h"
#include "Platform.h"
#include "Texture.h"

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

RMeshShape::RMeshShape(const string& Filename)
{
	string MeshFilename = Filename;
	ifstream InputMeshFile(MeshFilename);
	
	if (!InputMeshFile.is_open())
	{
		// If file is not found, search an alternative path for it.
		MeshFilename = std::string("../") + MeshFilename;
		InputMeshFile = ifstream(MeshFilename.c_str());

		if (!InputMeshFile.is_open())
		{
			RLog("Error - RMeshShape: Unable to open %s!\n", Filename.c_str());
			return;
		}
	}

	vector<string> MaterialNameList;
	int CurrentMaterialIdx = -1;

	RLog("Loading mesh from file: %s\n", MeshFilename.c_str());
	string Line;
	while (getline(InputMeshFile, Line))
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
		else if (key == "vt")
		{
			stringstream LineStream(Line);
			float t[3];
			LineStream >> dummy >> t[0] >> t[1];
			t[2] = 0.0f;
			Texcoords.push_back(RVec3(t));
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
                    
					int TexcoordIndex = GetNthNumericValue(1, Tokens[Index]) - 1;
					TexcoordIndices.push_back(TexcoordIndex);

                    int NormalIndex = GetNthNumericValue(2, Tokens[Index]) - 1;
                    NormalIndices.push_back(NormalIndex);

					if (i % 3 == 0)
					{
						// Store material id of each polygon
						PolyMaterialId.push_back(CurrentMaterialIdx);
					}
                }
            }
		}
		else if (key == "usemtl")
		{
			auto Tokens = Split(Line, ' ');
			string MaterialName = Tokens[1];

			auto Iter = find(MaterialNameList.begin(), MaterialNameList.end(), MaterialName);
			if (Iter == MaterialNameList.end())
			{
				MaterialNameList.push_back(MaterialName);
				CurrentMaterialIdx = (int)MaterialNameList.size() - 1;
			}
			else
			{
				CurrentMaterialIdx = (int)(Iter - MaterialNameList.begin());
			}
		}
	}

	InputMeshFile.close();
	RLog("Mesh loaded from %s. Verts: %d, Triangles: %d\n", Filename.c_str(), (int)Points.size(), (int)PointIndices.size() / 3);

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

	// Load materials from .mtl file
	string MaterialFilename = MeshFilename;
	auto Index = MaterialFilename.find(".obj");
	if (Index != string::npos)
	{
		MaterialFilename.replace(Index, 4, ".mtl");

		ifstream InputMaterialFile(MaterialFilename.c_str());
		if (InputMaterialFile.is_open())
		{
			string BasePath;

			auto Index = MaterialFilename.find_last_of("\\/");
			if (Index != string::npos)
			{
				BasePath = MaterialFilename.substr(0, Index + 1);
			}

			Textures.resize(PolyMaterialId.size());
			CurrentMaterialIdx = -1;

			while (getline(InputMaterialFile, Line))
			{
				string key = GetLineKeyword(Line);
				string Dummy;

				if (key == "newmtl")
				{
					stringstream LineStream(Line);
					string MaterialName;
					LineStream >> Dummy >> MaterialName;
					auto Iter = find(MaterialNameList.begin(), MaterialNameList.end(), MaterialName);
					if (Iter == MaterialNameList.end())
					{
						CurrentMaterialIdx = -1;
					}
					else
					{
						CurrentMaterialIdx = (int)(Iter - MaterialNameList.begin());
					}
				}
				else if (key == "map_Kd")
				{
					if (CurrentMaterialIdx == -1)
					{
						// This material is not used by the mesh
						continue;
					}

					stringstream LineStream(Line);
					string TexturePath;

					LineStream >> Dummy >> TexturePath;

					// Assuming texture path is relative
					TexturePath = BasePath + TexturePath;
                    
                    auto SlashIdx = TexturePath.find("\\\\");
                    while (SlashIdx != string::npos)
                    {
                        TexturePath.replace(SlashIdx, 2, "/");
                        SlashIdx = TexturePath.find("\\\\");
                    }

					Textures[CurrentMaterialIdx] = RTexture::LoadTexturePNG(TexturePath);
				}
			}

			InputMaterialFile.close();
		}
	}

	RLog("Generating spatial information for the mesh... ");
	Spatial = unique_ptr<KdTree>(new KdTree());
	Spatial->Build(Points.data(), PointIndices.data(), (int)PointIndices.size());
	RLog("Done\n");
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

				float u, v, w;
				RMath::Barycentric(p, a, b, c, u, v, w);

				const RVec3& n0 = Normals[NormalIndices[v0]];
				const RVec3& n1 = Normals[NormalIndices[v1]];
				const RVec3& n2 = Normals[NormalIndices[v2]];

                // Use fast inverse square root for approximating normal direction
				OutResult->HitNormal = (n0 * u + n1 * v + n2 * w).GetNormalizedVec3_Fast();

				int MaterialId = PolyMaterialId[TriangleIndex];
				if (MaterialId != -1 && MaterialId < (int)Textures.size())
				{
					RTexture* Texture = Textures[MaterialId].get();
					if (Texture)
					{
						const RVec3& t0 = Texcoords[TexcoordIndices[v0]];
						const RVec3& t1 = Texcoords[TexcoordIndices[v1]];
						const RVec3& t2 = Texcoords[TexcoordIndices[v2]];

						RVec3 texcoord = t0 * u + t1 * v + t2 * w;

						OutResult->SampledColor = Texture->Sample(texcoord.x, 1.0f - texcoord.y);
					}
				}
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
