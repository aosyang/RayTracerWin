
#include "RenderWindow.h"
#include <stdio.h>
#include "RRay.h"
#include <thread>
#include "Light.h"

#include "Math.h"
#include "ColorBuffer.h"
#include "Shapes.h"

#define USE_LIGHTS 0

typedef unsigned int Pixel;

Pixel bitcolor[bitmapWidth * bitmapHeight];

HDC g_DeviceContext;
void PresentRenderBuffer();


// Windows callback function
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		// share post quit message with WM_CLOSE
	case WM_CLOSE:
	{
		PostQuitMessage(0);
		break;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

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
	RVec3 LightDirection = InLight->Direction;
	float dist = 0.0f;

	switch (InLight->Type)
	{
	case LT_Point:
		LightDirection = (InLight->Position - InHitResult.HitPosition).GetNormalizedVec3();
		dist = (InHitResult.HitPosition - InLight->Position).Magnitude();
		break;

	case LT_Directional:
		LightDirection = InLight->Direction;
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
		float ldp = max(0.0f, InHitResult.HitNormal.Dot(LightDirection));
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

					RVec3 DiffuseReflectionDirection = RandomHemisphereDirection(result.HitNormal);
					RRay DiffuseRay(result.HitPosition + DiffuseReflectionDirection * 0.001f, DiffuseReflectionDirection, RemainingDistance);

					DotProductResult = max(0.0f, result.HitNormal.Dot(DiffuseReflectionDirection));

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

void PresentRenderBuffer()
{
	BITMAPINFO	info;
	ZeroMemory(&info, sizeof(BITMAPINFO));
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = bitmapWidth;
	info.bmiHeader.biHeight = -int(bitmapHeight); // flip
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	SetDIBitsToDevice(g_DeviceContext, 0, 0, bitmapWidth, bitmapHeight, 0, 0, 0, bitmapHeight, bitcolor, &info, DIB_RGB_COLORS);
}

void ThreadRender(int begin, int end, int MaxBounceCount = 10, const RenderOption& InOption = RenderOption())
{
	for (int i = begin; i < end; i++)
	{
		int x, y;
		BufferIndexToCoord(i, x, y);
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

		int sample = 1000;

		if (InOption.UseBaseColor)
		{
			sample = 1;
		}

		for (int i = 0; i < 4; i++)
		{
			float offset_x = ox[i];
			float offset_y = oy[i];

			// Randomize sampling point
			offset_x += (Random() - 0.5f) * offset;
			offset_y += (Random() - 0.5f) * offset;

			for (int j = 0; j < sample; j++)
			{
				RRay ray(RVec3(0, 0, -5), RVec3(dx + offset_x, dy + offset_y, 0.5f), 1000.0f);
				c += RayTrace(ray, MaxBounceCount, InOption);
			}
		}

		c /= 4.0f * sample;
		Pixel color = MakePixelColor(c);
#endif

		// ARGB
		*(bitcolor + i) = color;
	}
}

void UpdateBitmapPixels()
{
	RenderOption BaseColorOption;
	BaseColorOption.UseBaseColor = true;

	// Draw base color for preview
	ThreadRender(0, sizeof(bitcolor) / sizeof(Pixel), 1, BaseColorOption);

	//return;

	const int ThreadCount = 8;

	int step = sizeof(bitcolor) / sizeof(Pixel) / ThreadCount;

	for (int i = 0; i < ThreadCount; i++)
	{
#if 1
		std::thread t1(ThreadRender, i * step, (i + 1) * step, 10, RenderOption());
		t1.detach();
#else
		ThreadRender(i * step, (i + 1) * step);
		PresentRenderBuffer();
#endif
	}
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	SetProcessDPIAware();

	RenderWindow rw;
	rw.Create(WndProc, bitmapWidth, bitmapHeight);

	g_DeviceContext = GetDC(rw.GetHwnd());

	MSG msg;
	bool quit = false;

	DWORD nextTick = GetTickCount() + 1000;
	int frame = 0;

	std::thread RenderThread(UpdateBitmapPixels);
	RenderThread.detach();

	while (!quit)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				quit = true;
			}
			else
			{
				// Translate the message and dispatch it to WndProc()
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		Sleep(1);
		PresentRenderBuffer();

		frame++;
		DWORD t = GetTickCount();
		if (t >= nextTick)
		{
			nextTick = t + 1000;
#if 0
			char buf[1024];
			sprintf_s(buf, "FPS: %d\n", frame);
			OutputDebugStringA(buf);
#endif
			frame = 0;
		}
	}

	rw.Destroy();

	return 0;
}