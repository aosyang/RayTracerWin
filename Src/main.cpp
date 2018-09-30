//=============================================================================
// main.cpp by Shiyang Ao, 2017 All Rights Reserved.
//
// 
//=============================================================================
#include "Platform.h"

#if (PLATFORM_OSX)
#include "OSX/OSXWindow.h"
#elif (PLATFORM_WIN32)
#include "Windows/RenderWindow.h"
#endif

#include <stdio.h>
#include "RRay.h"
#include <thread>
#include "Light.h"

#include "Math.h"
#include "ColorBuffer.h"
#include "Shapes.h"

#include <vector>
#include <chrono>

#define USE_LIGHTS 0

Pixel bitcolor[bitmapWidth * bitmapHeight];

struct AccumulatePixel
{
	AccumulatePixel()
		: R(0), G(0), B(0), Num(0)
	{}

	void AddPixel(Pixel Color)
	{
		R += GetUint32ColorRed(Color);
		G += GetUint32ColorGreen(Color);
		B += GetUint32ColorBlue(Color);
		Num++;
	}

	Pixel GetPixel()
	{
		return MakeUint32Color((BYTE)(R / Num), (BYTE)(G / Num), (BYTE)(B / Num), 0xFF);
	}

	int R;
	int G;
	int B;
	int Num;
};

AccumulatePixel accuBuffer[bitmapWidth * bitmapHeight];
RenderWindow g_RenderWindow;

// Types of shapes used for ray tracing
enum EShape
{
	ST_Sphere,
	ST_Plane,
};

enum EMaterial
{
	MT_Emissive		= 1 << 0,
	MT_Diffuse		= 1 << 1,
	MT_Reflective	= 1 << 2,
};

struct RenderOption
{
	bool UseBaseColor;

	RenderOption()
		: UseBaseColor(false)
	{}
};

// Data structure of a shape in the scene
struct ShapeData
{
	RShape*	Shape;
	RVec3	Color;
	bool	UseCheckerboardPattern;
	int		MaterialFlags;
};

ShapeData GSceneShapes[] =
{
	{ RSphere::Create(RVec3(0.0f, -1.0f, 2.0f) ,   1.0f), RVec3(1.0f, 0.5f, 0.1f), false, MT_Diffuse | MT_Reflective },
	//{ RSphere::Create(RVec3(1.5f, 0.0f, 2.0f) ,    0.5f), RVec3(0.0f, 10.0f, 10.0f), false, MT_Emissive/*MT_Reflective*/ },
	{ RSphere::Create(RVec3(1.2f, 1.0f, 3.0f) ,    0.5f), RVec3(0.1f, 1.0f, 0.2f), false, MT_Diffuse },
	{ RSphere::Create(RVec3(0.2f, 1.2f, 1.0f) ,    0.5f), RVec3(0.5f, 0.0f, 0.2f), false, MT_Diffuse | MT_Reflective },
	{ RSphere::Create(RVec3(-2.8f, 1.2f, 4.0f) ,   1.5f), RVec3(0.95f, 0.75f, 0.1f), false, MT_Diffuse | MT_Reflective/*MT_Reflective*/ },
	{ RSphere::Create(RVec3(0.0f, -5.0f, 0.0f) ,    0.5f), RVec3(100.0f, 100.0f, 120.0f), false, MT_Emissive },	// Ceiling light
	//{ RSphere::Create(RVec3(0.0f, 0.0f, -1005.0f) , 1000.0f), RVec3(0.0f, 0.5f, 0.75f), MT_Emissive },
	//{ RSphere::Create(RVec3(1055.0f, 0.0f, 0.0f) , 1000.0f), 0xFF007FBF, MT_Emissive },
	//{ RSphere::Create(RVec3(-1055.0f, 0.0f, 0.0f) , 1000.0f), 0xFF007FBF, MT_Emissive },

	{ RCapsule::Create(RVec3(1.5f, 0.0f, 0.0f), RVec3(2.0f, 1.0f, 0.0f), 0.5f), RVec3(0.25f, 0.75f, 0.6f), false, MT_Diffuse | MT_Reflective },

	{ RPlane::Create(RVec3(0.0f, -1.0f, 0.0f) ,    RVec3(0.0f, 2.5f, 0.0f)), RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse },			// Ground
	{ RPlane::Create(RVec3(0.0f, 1.0f, 0.0f) ,    RVec3(0.0f, -5.0f, 0.0f)), RVec3(1.2f, 1.2f, 1.5f), false, MT_Diffuse },			// Ceiling / Sky light plane
	{ RPlane::Create(RVec3(0.0f, 0.0f, -1.0f) ,    RVec3(0.0f, 0.0f, 5.0f)), RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse },			// Back wall
	{ RPlane::Create(RVec3(0.0f, 0.0f, 1.0f) ,    RVec3(0.0f, 0.0f, -10.0f)), RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse },
	{ RPlane::Create(RVec3(-1.0f, 0.0f, 0.0f) ,    RVec3(5.0f, 0.0f, 0.0f)), RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse },			// Right wall
	{ RPlane::Create(RVec3(1.0f, 0.0f, 0.0f) ,    RVec3(-5.0f, 0.0f, 0.0f)), RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse },			// Left wall
	//{ RVec3(-1005.0f, 0.0f, 0.0f) , 1000.0f, 0xFFFF0000, false },
	//{ RVec3(1005.0f, 0.0f, 0.0f) , 1000.0f, 0xFF7FBF00, false },
	//{ RVec3(0.0f, 0.0f, -995.0f) , 1000.0f, 0xFFFF0000, false },
};

LightData GSceneLights[] =
{
	//{ LT_Directional,	RVec3(0.0f, -1.0f, 0.0f), RVec3(1, 1, 1) },
	{ LT_Point,			RVec3(0.0f, -4.5f, 0.0f), RVec3(1, 1, 1) },
};


RVec3 CalculateLightColor(const LightData* InLight, const RayHitResult &InHitResult, const RVec3& InSurfaceColor)
{
	RVec3 LightDirection = InLight->PositionOrDirection;
	float dist = 0.0f;

	switch (InLight->Type)
	{
	case LT_Point:
		{
			RVec3 LightPosition = InLight->PositionOrDirection;
			LightDirection = (LightPosition - InHitResult.HitPosition).GetNormalizedVec3();
			dist = (InHitResult.HitPosition - LightPosition).Magnitude();
		}
		break;

	case LT_Directional:
		LightDirection = InLight->PositionOrDirection;
		dist = 1000.0f;
		break;
	}

	RRay ShadowRay(InHitResult.HitPosition + LightDirection * 0.001f, LightDirection, dist);
	bool IsInShadow = false;

	// Check if light path has been blocked by any shapes
	for (int i = 0; i < ARRAYSIZE(GSceneShapes); i++)
	{
		bool hit = GSceneShapes[i].Shape->TestRayIntersection(ShadowRay);

		if (hit)
		{
			IsInShadow = true;
			break;
		}
	}

	if (IsInShadow)
	{
		// Surface is in shadow, no light contribution
		return RVec3::Zero();
	}
	else
	{
        float ldp = Math::Max(0.0f, InHitResult.HitNormal.Dot(LightDirection));
		return InSurfaceColor * ldp;
	}
}

RVec3 RayTrace(const RRay& InRay, int MaxBounceTimes = 10, const RenderOption& InOption = RenderOption())
{
	RRay TestRay = InRay;

	if (MaxBounceTimes == 0)
		return RVec3::Zero();

	RVec3 FinalColor = RVec3::Zero();

	RayHitResult result;
	ShapeData* hitShape = nullptr;

	// Get nearest hit point for this ray
	for (int i = 0; i < ARRAYSIZE(GSceneShapes); i++)
	{
		bool hit = GSceneShapes[i].Shape->TestRayIntersection(TestRay, &result);

		if (hit)
		{
			// Shorten distance of current testing ray
			TestRay.Distance = result.Distance;
			hitShape = &GSceneShapes[i];
		}
	}

	if (hitShape)
	{
		float DiffuseRatio = 1.0f;

		if (hitShape->MaterialFlags & MT_Reflective)
		{
			float RemainingDistance = InRay.Distance - result.Distance;
			if (RemainingDistance > 0)
			{
				const float ReflectiveRatio = 0.5f;
				DiffuseRatio = 1.0f - ReflectiveRatio;

				if (!InOption.UseBaseColor)
				{
					if (InRay.Direction.Dot(result.HitNormal) < 0)
					{
						RVec3 newDir = InRay.Direction.Reflect(result.HitNormal);

						RRay reflRay(result.HitPosition + newDir * 0.001f, newDir, RemainingDistance);

						FinalColor += RayTrace(reflRay, MaxBounceTimes - 1, InOption) * ReflectiveRatio;
					}
				}
			}
		}

		RVec3 SurfaceColor = hitShape->Color;

		// Make checkerboard pattern
		if (hitShape->UseCheckerboardPattern)
		{
			bool color = false;
			float fx = result.HitPosition.x * 0.2f;
			float fy = result.HitPosition.y * 0.2f;
			float fz = result.HitPosition.z * 0.2f;

			if (fx - floorf(fx) > 0.5f)
				color = !color;
			if (fz - floorf(fz) > 0.5f)
				color = !color;
			if (fy - floorf(fy) > 0.5f)
				color = !color;

			if (!color)
				SurfaceColor *= 0.5f;
		}

		if (hitShape->MaterialFlags & MT_Diffuse)
		{
			float RemainingDistance = InRay.Distance - result.Distance;
			if (RemainingDistance > 0)
			{
				float DotProductResult = 1.0f;
				RVec3 DiffuseColor = RVec3(1.0f, 1.0f, 1.0f);

				if (!InOption.UseBaseColor)
				{

					RVec3 DiffuseReflectionDirection = RMath::RandomHemisphereDirection(result.HitNormal);
					RRay DiffuseRay(result.HitPosition + DiffuseReflectionDirection * 0.001f, DiffuseReflectionDirection, RemainingDistance);

                    DotProductResult = Math::Max(0.0f, result.HitNormal.Dot(DiffuseReflectionDirection));

					DiffuseColor = RayTrace(DiffuseRay, MaxBounceTimes - 1, InOption);

#if (USE_LIGHTS == 1)
					// Apply diffuse lighting
					for (int i = 0; i < ARRAYSIZE(GSceneLights); i++)
					{
						LightData* l = &GSceneLights[i];
						FinalColor += CalculateLightColor(l, result, SurfaceColor) * DiffuseRatio;
					}
#endif
				}

				FinalColor += SurfaceColor * DiffuseColor * DotProductResult * DiffuseRatio;
			}
		}

		if (hitShape->MaterialFlags & MT_Emissive)
		{
			FinalColor += SurfaceColor;
		}
	}

	return FinalColor;
}

void ThreadWorker_Render(int begin, int end, int MaxBounceCount = 10, const RenderOption& InOption = RenderOption())
{
//#pragma omp parallel for schedule(dynamic, 1)
	for (int PixelIndex = begin; PixelIndex < end; PixelIndex++)
	{
		int x, y;
		BufferIndexToCoord(PixelIndex, x, y);
		float dx = (float)(x - bitmapWidth / 2) / (bitmapWidth * 2);
		float dy = (float)(y - bitmapWidth / 2) / (bitmapHeight * 2);

#if 0
		RRay ray(RVec3(0, 0, -3), RVec3(dx, dy, 0.5f), 1000.0f);

		Pixel color = MakePixelColor(RayTrace(ray));
#else
		float inv_pixel_width = 1.0f / (bitmapWidth * 4);

		float ox[4] = { 0.0f, inv_pixel_width, 0.0f, inv_pixel_width };
		float oy[4] = { 0.0f, 0.0f, inv_pixel_width, inv_pixel_width };

		const float offset = inv_pixel_width * 0.5f;

		RVec3 c = RVec3::Zero();

		// Randomly sample 4x4 nearby pixels for antialiasing
		for (int i = 0; i < 4; i++)
		{
			float offset_x = ox[i];
			float offset_y = oy[i];

			// Randomize sampling point
			offset_x += (RMath::Random() - 0.5f) * offset;
			offset_y += (RMath::Random() - 0.5f) * offset;

			RVec3 Dir(dx + offset_x, dy + offset_y, 0.5f);
			RRay ray(RVec3(0, 0, -5), Dir.GetNormalizedVec3(), 1000.0f);
			c += RayTrace(ray, MaxBounceCount, InOption);
		}

		c /= 4.0f;
		Pixel color = MakePixelColor(c);
#endif

		if (InOption.UseBaseColor)
		{
			// ARGB
			*(bitcolor + PixelIndex) = color;
		}
		else
		{
			accuBuffer[PixelIndex].AddPixel(color);
			*(bitcolor + PixelIndex) = accuBuffer[PixelIndex].GetPixel();
		}
	}
}

void UpdateBitmapPixels()
{
	RenderOption BaseColorOption;
	BaseColorOption.UseBaseColor = true;

	// Draw base color for preview
	ThreadWorker_Render(0, sizeof(bitcolor) / sizeof(Pixel), 1, BaseColorOption);

	//return;

	const int ThreadCount = 8;

	int step = sizeof(bitcolor) / sizeof(Pixel) / ThreadCount;

	std::vector<std::thread> RenderThreads;
	static const int TotalSamplesNum = 500;

	auto StartTime = std::chrono::system_clock::now();

	for (int Sample = 0; Sample < TotalSamplesNum; Sample++)
	{
		for (int i = 0; i < ThreadCount; i++)
		{
#if 1
			RenderThreads.push_back(std::thread(ThreadWorker_Render, i * step, (i + 1) * step, 10, RenderOption()));
#else
			ThreadWorker_Render(i * step, (i + 1) * step);
			PresentRenderBuffer(g_DeviceContext);
#endif
		}

		// Wait until all threads finish their work of current sample
		for (auto& Thread : RenderThreads)
		{
			Thread.join();
		}

		RenderThreads.clear();

		auto CurrentTime = std::chrono::system_clock::now();
		auto ElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - StartTime);
		auto RemainingTime = ElapsedTime / (Sample + 1) * (TotalSamplesNum - Sample - 1);
		int ElapsedTimeSec = (int)(ElapsedTime.count() / 1000);
		int RemainingTimeSec = (int)(RemainingTime.count() / 1000);

		char buf[1024];
		sprintf_s(buf, sizeof(buf), "Ray Tracer - S: [%d/%d] | T: [%ds/%ds]", Sample + 1, TotalSamplesNum, ElapsedTimeSec, RemainingTimeSec);
		g_RenderWindow.SetTitle(buf);
	}
}

#if (PLATFORM_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, char *argv[])
#endif
{
	RMath::InitPseudoRandomUnitVector();

	g_RenderWindow.Create(bitmapWidth, bitmapHeight);
	g_RenderWindow.SetRenderBufferParameters(bitmapWidth, bitmapHeight, bitcolor);

    // Begin ray tracing render thread
	std::thread RenderThread(UpdateBitmapPixels);
	RenderThread.detach();

	g_RenderWindow.RunWindowLoop();

	g_RenderWindow.Destroy();

	return 0;
}
