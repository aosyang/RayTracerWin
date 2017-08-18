
#include "RenderWindow.h"
#include <stdio.h>
#include "RRay.h"
#include <thread>
#include "Light.h"

#include "Math.h"
#include "ColorBuffer.h"

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

// Data structure of a shape in the scene
struct ShapeData
{
	EShape	type;
	RVec3	c;
	RVec3	r;
	RVec3	Color;
	bool	UseCheckerboardPattern;
	int		MaterialFlags;
};

ShapeData GSceneShapes[] =
{
	{ ST_Sphere, RVec3(0.0f, -1.0f, 2.0f) ,   RVec3(1.0f, 0.0f, 0.0f), RVec3(1.0f, 0.5f, 0.1f), false, MT_Diffuse | MT_Reflective },
	//{ ST_Sphere, RVec3(1.5f, 0.0f, 2.0f) ,    RVec3(0.5f, 0.0f, 0.0f), RVec3(0.0f, 10.0f, 10.0f), false, MT_Emissive/*MT_Reflective*/ },
	{ ST_Sphere, RVec3(1.2f, 1.0f, 3.0f) ,    RVec3(0.5f, 0.0f, 0.0f), RVec3(0.1f, 1.0f, 0.2f), false, MT_Diffuse },
	{ ST_Sphere, RVec3(0.2f, 1.2f, 1.0f) ,    RVec3(0.5f, 0.0f, 0.0f), RVec3(0.5f, 0.0f, 0.2f), false, MT_Diffuse | MT_Reflective },
	{ ST_Sphere, RVec3(-2.8f, 1.2f, 4.0f) ,   RVec3(1.5f, 0.0f, 0.0f), RVec3(0.95f, 0.75f, 0.1f), false, MT_Diffuse | MT_Reflective/*MT_Reflective*/ },
	{ ST_Sphere, RVec3(0.0f, -5.0f, 0.0f) ,    RVec3(0.5f, 0.0f, 0.0f), RVec3(100.0f, 100.0f, 120.0f), false, MT_Emissive },	// Ceiling light
	//{ ST_Sphere, RVec3(0.0f, 0.0f, -1005.0f) , RVec3(1000.0f, 0.0f, 0.0f), RVec3(0.0f, 0.5f, 0.75f), MT_Emissive },
	//{ ST_Sphere, RVec3(1055.0f, 0.0f, 0.0f) , RVec3(1000.0f, 0.0f, 0.0f), 0xFF007FBF, MT_Emissive },
	//{ ST_Sphere, RVec3(-1055.0f, 0.0f, 0.0f) , RVec3(1000.0f, 0.0f, 0.0f), 0xFF007FBF, MT_Emissive },

	{ ST_Plane, RVec3(0.0f, -1.0f, 0.0f) ,    RVec3(0.0f, 2.5f, 0.0f), RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse },			// Ground
	{ ST_Plane, RVec3(0.0f, 1.0f, 0.0f) ,    RVec3(0.0f, -5.0f, 0.0f), RVec3(1.2f, 1.2f, 1.5f), false, MT_Diffuse },		// Ceiling / Sky light plane
	{ ST_Plane, RVec3(0.0f, 0.0f, -1.0f) ,    RVec3(0.0f, 0.0f, 5.0f), RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse },			// Back wall
	{ ST_Plane, RVec3(0.0f, 0.0f, 1.0f) ,    RVec3(0.0f, 0.0f, -10.0f), RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse },
	{ ST_Plane, RVec3(-1.0f, 0.0f, 0.0f) ,    RVec3(5.0f, 0.0f, 0.0f), RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse },			// Right wall
	{ ST_Plane, RVec3(1.0f, 0.0f, 0.0f) ,    RVec3(-5.0f, 0.0f, 0.0f), RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse },			// Left wall
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
		bool hit = false;

		switch (GSceneShapes[i].type)
		{
		case ST_Sphere:
			hit = ShadowRay.TestIntersectionWithSphere(GSceneShapes[i].c, GSceneShapes[i].r.x);
			break;
		case ST_Plane:
			hit = ShadowRay.TestIntersectionWithPlane(GSceneShapes[i].c, GSceneShapes[i].r);
			break;
		};

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

RVec3 RayTrace(const RRay& InRay, int BouncedTimes = 0)
{
	RRay TestRay = InRay;

	if (BouncedTimes > 10)
		return RVec3::Zero();

	RVec3 FinalColor = RVec3::Zero();

	RayHitResult result;
	ShapeData* hitShape = nullptr;

	// Get nearest hit point for this ray
	for (int i = 0; i < ARRAYSIZE(GSceneShapes); i++)
	{
		bool hit = false;
		
		switch (GSceneShapes[i].type)
		{
		case ST_Sphere:
			hit = TestRay.TestIntersectionWithSphere(GSceneShapes[i].c, GSceneShapes[i].r.x, &result);
			break;
		case ST_Plane:
			hit = TestRay.TestIntersectionWithPlane(GSceneShapes[i].c, GSceneShapes[i].r, &result);
			break;
		};

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
				if (InRay.Direction.Dot(result.HitNormal) < 0)
				{
					RVec3 newDir = InRay.Direction.Reflect(result.HitNormal);

					RRay reflRay(result.HitPosition + newDir * 0.001f, newDir, RemainingDistance);

					const float ReflectiveRatio = 0.5f;
					DiffuseRatio = 1.0f - ReflectiveRatio;
					FinalColor += RayTrace(reflRay, BouncedTimes + 1) * ReflectiveRatio;
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
				RVec3 DiffuseReflectionDirection = RandomHemisphereDirection(result.HitNormal);
				RRay DiffuseRay(result.HitPosition + DiffuseReflectionDirection * 0.001f, DiffuseReflectionDirection, RemainingDistance);

				float DotProductResult = max(0.0f, result.HitNormal.Dot(DiffuseReflectionDirection));

				RVec3 DiffuseColor = RayTrace(DiffuseRay, BouncedTimes + 1);

#if (USE_LIGHTS == 1)
				// Apply diffuse lighting
				for (int i = 0; i < ARRAYSIZE(GSceneLights); i++)
				{
					LightData* l = &GSceneLights[i];
					FinalColor += CalculateLightColor(l, result, SurfaceColor) * DiffuseRatio;
				}
#endif

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

void ThreadRender(int begin, int end)
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

		const int sample = 1000;

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
				c += RayTrace(ray);
			}
		}

		c /= 4 * sample;
		Pixel color = MakePixelColor(c);
#endif

		// ARGB
		*(bitcolor + i) = color;
	}
}

void UpdateBitmapPixels()
{
	DWORD tick = GetTickCount();

	int step = sizeof(bitcolor) / sizeof(Pixel) / 8;

	for (int i = 0; i < 8; i++)
	{
#if 1
		std::thread t1(ThreadRender, i * step, (i + 1) * step);
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
	UpdateBitmapPixels();

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
			char buf[1024];
			sprintf_s(buf, "FPS: %d\n", frame);
			OutputDebugStringA(buf);
			frame = 0;
		}
	}

	rw.Destroy();

	return 0;
}