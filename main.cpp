
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


enum ShapeType
{
	ST_Sphere,
	ST_Plane,
};

enum MaterialType
{
	MT_Emissive		= 1 << 0,
	MT_Diffuse		= 1 << 1,
	MT_Reflective	= 1 << 2,
};

struct Shape
{
	ShapeType type;
	RVec3 c;
	RVec3 r;
	RVec3 color;
	bool pattern;
	int matType;
};

Shape shapes[] =
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

Light lights[] =
{
	//{ LT_Directional,	RVec3(0.0f, -1.0f, 0.0f), RVec3(1, 1, 1) },
	{ LT_Point,			RVec3(0.0f, -4.5f, 0.0f), RVec3(1, 1, 1) },
};


RVec3 CalculateLightColor(const Light* light, RayHitResult &result, const RVec3& surface_color)
{
	RVec3 light_dir = light->pos_dir;
	float dist = 0.0f;

	switch (light->type)
	{
	case LT_Point:
		light_dir = (light->pos_dir - result.hitPoint).GetNormalizedVec3();
		dist = (result.hitPoint - light->pos_dir).Magnitude();
		break;

	case LT_Directional:
		light_dir = light->pos_dir;
		dist = 1000.0f;
		break;
	}

	RRay shadow_ray(result.hitPoint + light_dir * 0.001f, light_dir, dist);
	bool shadow = false;

	// Check if light path has been blocked by any shapes
	for (int i = 0; i < ARRAYSIZE(shapes); i++)
	{
		bool hit = false;

		switch (shapes[i].type)
		{
		case ST_Sphere:
			hit = shadow_ray.TestSphereIntersection(shapes[i].c, shapes[i].r.x);
			break;
		case ST_Plane:
			hit = shadow_ray.TestPlaneIntersection(shapes[i].c, shapes[i].r);
			break;
		};

		if (hit)
		{
			shadow = true;
			break;
		}
	}

	if (!shadow)
	{
		float ldp = max(0.0f, result.hitNormal.Dot(light_dir));
		return surface_color * ldp;
	}

	return RVec3::Zero();
}

RVec3 RayTrace(const RRay& ray, int refl_count = 0)
{
	RRay newRay = ray;

	if (refl_count > 10)
		return RVec3::Zero();

	RVec3 c = RVec3::Zero();

	RayHitResult result;
	Shape* hitShape = nullptr;

	// Get nearest hit point for this ray
	for (int i = 0; i < ARRAYSIZE(shapes); i++)
	{
		bool hit = false;
		
		switch (shapes[i].type)
		{
		case ST_Sphere:
			hit = newRay.TestSphereIntersection(shapes[i].c, shapes[i].r.x, &result);
			break;
		case ST_Plane:
			hit = newRay.TestPlaneIntersection(shapes[i].c, shapes[i].r, &result);
			break;
		};

		if (hit)
		{
			// Shorten distance of current testing ray
			newRay.Distance = result.dist;
			hitShape = &shapes[i];
		}
	}

	if (hitShape)
	{
		float diff_ratio = 1.0f;

		if (hitShape->matType & MT_Reflective)
		{
			float dist = ray.Distance - result.dist;
			if (dist > 0)
			{
				if (ray.Direction.Dot(result.hitNormal) < 0)
				{
					RVec3 newDir = ray.Direction.Reflect(result.hitNormal);

					RRay reflRay(result.hitPoint + newDir * 0.001f, newDir, dist);

					const float refl_ratio = 0.5f;
					diff_ratio = 1.0f - refl_ratio;
					c += RayTrace(reflRay, refl_count + 1) * refl_ratio;
				}
			}
		}

		RVec3 surface_color = hitShape->color;

		// Make checkerboard pattern
		if (hitShape->pattern)
		{
			bool color = false;
			float fx = result.hitPoint.x * 0.2f;
			float fy = result.hitPoint.y * 0.2f;
			float fz = result.hitPoint.z * 0.2f;

			if (fx - floorf(fx) > 0.5f)
				color = !color;
			if (fz - floorf(fz) > 0.5f)
				color = !color;
			if (fy - floorf(fy) > 0.5f)
				color = !color;

			if (!color)
				surface_color /= 2.0f;
		}

		if (hitShape->matType & MT_Diffuse)
		{
			float dist = ray.Distance - result.dist;
			if (dist > 0)
			{
				RVec3 newDir = RandomHemisphereDir(result.hitNormal);
				RRay diff_ray(result.hitPoint + newDir * 0.001f, newDir, dist);

				float dp = max(0.0f, result.hitNormal.Dot(newDir));

				RVec3 diff_refl = RayTrace(diff_ray, refl_count + 1);

#if (USE_LIGHTS == 1)
				// Apply diffuse lighting
				for (int i = 0; i < ARRAYSIZE(lights); i++)
				{
					Light* l = &lights[i];
					c += CalculateLightColor(l, result, surface_color) * diff_ratio;
				}
#endif

				c += surface_color * diff_refl * dp * diff_ratio;
			}
		}

		if (hitShape->matType & MT_Emissive)
		{
			c += surface_color;
		}
	}

	return c;
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

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
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