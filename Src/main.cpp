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
#include "MeshShape.h"
#include "RayTracerScene.h"

#include "ThreadTaskQueue.h"

#include <vector>
#include <chrono>

Pixel bitcolor[bitmapWidth * bitmapHeight];

struct AccumulatePixel
{
	AccumulatePixel()
		: AccumulatedColor(0, 0, 0), Num(0)
	{}

	void AddPixel(RVec3 Color)
	{
		AccumulatedColor += Color;
		Num++;
	}

	Pixel GetPixel() const
	{
		return MakePixelColor(AccumulatedColor / (float)Num);
	}

	Pixel GetGammaSpacePixel() const
	{
		return MakePixelColor(LinearToGamma(AccumulatedColor / (float)Num));
	}

	RVec3 AccumulatedColor;
	int Num;
};

AccumulatePixel accuBuffer[bitmapWidth * bitmapHeight];
RenderWindow g_RenderWindow;

struct RenderThreadTask
{
	RenderThreadTask()
	{
	}

	RenderThreadTask(int InStart, int InEnd, const RenderOption& InOption)
		: Start(InStart)
		, End(InEnd)
		, Option(InOption)
	{
	}

	int Start;
	int End;
	RenderOption Option;
};

ThreadTaskQueue<RenderThreadTask> g_TaskQueue;

RayTracerScene g_Scene;

void SetupScene()
{
	g_Scene.AddShape(RSphere::Create(RVec3(0.0f, -1.0f, 2.0f), 1.0f), RMaterial(RVec3(1.0f, 0.5f, 0.1f), false, MT_Diffuse | MT_Reflective));
	g_Scene.AddShape(RSphere::Create(RVec3(1.2f, 1.0f, 3.0f), 0.5f), RMaterial(RVec3(0.1f, 1.0f, 0.2f), false, MT_Diffuse));
	g_Scene.AddShape(RSphere::Create(RVec3(0.2f, 1.2f, 1.0f), 0.5f), RMaterial(RVec3(0.5f, 0.0f, 0.2f), false, MT_Diffuse | MT_Reflective));
	g_Scene.AddShape(RSphere::Create(RVec3(-2.8f, 1.2f, 4.0f), 1.5f), RMaterial(RVec3(0.95f, 0.75f, 0.1f), false, MT_Diffuse | MT_Reflective/*MT_Reflective*/));
	g_Scene.AddShape(RSphere::Create(RVec3(0.0f, -5.0f, 0.0f), 0.5f), RMaterial(RVec3(50.0f, 50.0f, 60.0f), false, MT_Emissive));					// Ceiling light
	g_Scene.AddShape(RCapsule::Create(RVec3(1.5f, 0.0f, 0.0f), RVec3(2.0f, 1.0f, 0.0f), 0.5f), RMaterial(RVec3(0.25f, 0.75f, 0.6f), false, MT_Diffuse | MT_Reflective));
	g_Scene.AddShape(RPlane::Create(RVec3(0.0f, -1.0f, 0.0f), RVec3(0.0f, 2.5f, 0.0f)), RMaterial(RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse));			// Ground
	g_Scene.AddShape(RPlane::Create(RVec3(0.0f, 1.0f, 0.0f), RVec3(0.0f, -5.0f, 0.0f)), RMaterial(RVec3(1.2f, 1.2f, 1.5f), false, MT_Diffuse));			// Ceiling / Sky light plane
	g_Scene.AddShape(RPlane::Create(RVec3(0.0f, 0.0f, -1.0f), RVec3(0.0f, 0.0f, 5.0f)), RMaterial(RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse));			// Back wall
	g_Scene.AddShape(RPlane::Create(RVec3(0.0f, 0.0f, 1.0f), RVec3(0.0f, 0.0f, -10.0f)), RMaterial(RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse));
	g_Scene.AddShape(RPlane::Create(RVec3(-1.0f, 0.0f, 0.0f), RVec3(5.0f, 0.0f, 0.0f)), RMaterial(RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse));			// Right wall
	g_Scene.AddShape(RPlane::Create(RVec3(1.0f, 0.0f, 0.0f), RVec3(-5.0f, 0.0f, 0.0f)), RMaterial(RVec3(1.0f, 1.0f, 1.0f), true, MT_Diffuse));			// Left wall

	g_Scene.AddShape(RMeshShape::Create("../Data/TorusKnot.obj"), RMaterial(RVec3(1.0f, 1.0f, 0.5f), false, MT_Diffuse | MT_Reflective));
}

void ThreadWorker_Render(int begin, int end, int MaxBounceCount = 10, const RenderOption& InOption = RenderOption())
{
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

		// Randomly sample 2x2 nearby pixels for antialiasing
		for (int i = 0; i < 4; i++)
		{
			float offset_x = ox[i];
			float offset_y = oy[i];

			// Randomize sampling point
			offset_x += (RMath::Random() - 0.5f) * offset;
			offset_y += (RMath::Random() - 0.5f) * offset;

			RVec3 Dir(dx + offset_x, dy + offset_y, 0.5f);
			RRay ray(RVec3(0, 0, -5), Dir.GetNormalizedVec3(), 1000.0f);
			c += g_Scene.RayTrace(ray, MaxBounceCount, InOption);
		}

		c /= 4.0f;
#endif

		if (InOption.UseBaseColor)
		{
			// ARGB
			Pixel color = MakePixelColor(LinearToGamma(c));
			*(bitcolor + PixelIndex) = color;
		}
		else
		{
			accuBuffer[PixelIndex].AddPixel(c);
			*(bitcolor + PixelIndex) = accuBuffer[PixelIndex].GetGammaSpacePixel();
		}
	}
}

void ThreadTaskWorker()
{
	RenderThreadTask Task;

	// Run until all tasks are finished
	while (g_TaskQueue.GetTask(&Task) && !g_Scene.IsTerminatingProgram())
	{
		ThreadWorker_Render(Task.Start, Task.End, 4, Task.Option);
	}
}

// Convert milliseconds to h:m:s format
void FormatTimeString(char* Buffer, int BufferSize, int Milliseconds)
{
	if (Milliseconds < 1000)
	{
		RPrintf(Buffer, BufferSize, "%dms", Milliseconds);
	}
	else
	{
		int Hours = Milliseconds / (3600 * 1000);
		int Minutes = Milliseconds / (60 * 1000) - Hours * 60;
		int Seconds = Milliseconds / 1000 - Minutes * 60 - Hours * 3600;

		if (Hours)
		{
			RPrintf(Buffer, BufferSize, "%dh:%dm:%ds", Hours, Minutes, Seconds);
		}
		else if (Minutes)
		{
			RPrintf(Buffer, BufferSize, "%dm:%ds", Minutes, Seconds);
		}
		else
		{
			RPrintf(Buffer, BufferSize, "%ds", Seconds);
		}
	}
}

void UpdateBitmapPixels()
{
	// Total number of worker threads
	const int ThreadCount = 8;

	RenderOption BaseColorOption;
	BaseColorOption.UseBaseColor = true;

	// Draw base color for preview
	{
		std::vector<std::thread> RenderThreads;

		// Split rendering area to tasks
		for (int i = 0; i < bitmapHeight; i++)
		{
			g_TaskQueue.AddTask(RenderThreadTask(i * bitmapWidth, (i + 1) * bitmapWidth - 1, BaseColorOption));
		}

		// Start all worker threads
		for (int i = 0; i < ThreadCount; i++)
		{
			RenderThreads.push_back(std::thread(ThreadTaskWorker));
		}

		// Wait until all threads finish their work of current sample
		for (auto& Thread : RenderThreads)
		{
			Thread.join();
		}

		//return;
	}

	// Number of times each pixel is sampled
	static const int TotalSamplesNum = 500;

	auto StartTime = std::chrono::system_clock::now();
	auto LastFrameTime = StartTime;

	for (int Sample = 0; Sample < TotalSamplesNum; Sample++)
	{
		std::vector<std::thread> RenderThreads;

		// Split rendering area to tasks
		for (int i = 0; i < bitmapHeight; i++)
		{
			g_TaskQueue.AddTask(RenderThreadTask(i * bitmapWidth, (i + 1) * bitmapWidth - 1, RenderOption()));
		}
		
		for (int i = 0; i < ThreadCount; i++)
		{
#if 1
			RenderThreads.push_back(std::thread(ThreadTaskWorker));
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

		auto CurrentTime = std::chrono::system_clock::now();
		auto ElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - StartTime);
		auto RemainingTime = ElapsedTime / (Sample + 1) * (TotalSamplesNum - Sample - 1);
		int ElapsedTimeMs = (int)ElapsedTime.count();
		int RemainingTimeMs = (int)RemainingTime.count();
		int FrameTimeMs = (int)std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - LastFrameTime).count();

		char ElapsedTimeStr[1024];
		FormatTimeString(ElapsedTimeStr, sizeof(ElapsedTimeStr), ElapsedTimeMs);

		char RemainingTimeStr[1024];
		FormatTimeString(RemainingTimeStr, sizeof(RemainingTimeStr), RemainingTimeMs);

		char TextBuffer[1024];
		RPrintf(TextBuffer, sizeof(TextBuffer), "RayTracer - S: [%d/%d] | T: [%s / %s] | F: [%dms]", Sample + 1, TotalSamplesNum, ElapsedTimeStr, RemainingTimeStr, FrameTimeMs);

		LastFrameTime = CurrentTime;

		// Log render information
        RLog("%s\n", TextBuffer);

		// Update window title with render information
		g_RenderWindow.SetTitle(TextBuffer);

		if (g_Scene.IsTerminatingProgram())
		{
			break;
		}
	}
}

#if (PLATFORM_WIN32)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, char *argv[])
#endif
{
	srand((unsigned int)time(nullptr));

	RMath::InitPseudoRandomUnitVector();

	g_RenderWindow.Create(bitmapWidth, bitmapHeight);
	g_RenderWindow.SetRenderBufferParameters(bitmapWidth, bitmapHeight, bitcolor);

	SetupScene();

    // Begin ray tracing render thread
	std::thread RenderThread(UpdateBitmapPixels);

	g_RenderWindow.RunWindowLoop();
	g_RenderWindow.Destroy();
	g_Scene.NotifyTerminatingProgram();

	RenderThread.join();

	return 0;
}
