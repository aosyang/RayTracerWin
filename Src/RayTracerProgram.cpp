//=============================================================================
// RayTracerProgram.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "RayTracerProgram.h"

#include <stdio.h>
#include "RRay.h"
#include "Light.h"

#include "Math.h"
#include "ColorBuffer.h"
#include "Shapes.h"
#include "MeshShape.h"
#include "RayTracerScene.h"

#include "ThreadTaskQueue.h"
#include "ThreadUtils.h"

#include <vector>
#include <chrono>
#include <sstream>

template<typename T, typename ...Args>
std::unique_ptr<T> MakeUnique(Args&&... args)
{
    return std::move(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
}

RayTracerProgram* RayTracerProgram::CurrentInstance = nullptr;

// Whether to enable 2x2 antialiasing for pixel sampling
#define ENABLE_ANTIALIASING 1

// Number of times each pixel is sampled
static const int TotalSamplesNum = 500;

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

typedef ThreadTaskQueue<RenderThreadTask> RenderThreadTaskQueue;

//////////////////////////////////////////////////////////////////////////
// Log thread - Begin
//////////////////////////////////////////////////////////////////////////

const auto ProgramStartTime = std::chrono::system_clock::now();

int GetTimeInMillisecond()
{
	auto CurrentTime = std::chrono::system_clock::now();
	return (int)std::chrono::duration_cast<std::chrono::milliseconds>(CurrentTime - ProgramStartTime).count();
}

void DisplayThreadAndTime()
{
	std::thread::id this_id = std::this_thread::get_id();
	std::stringstream ss;
	ss << this_id;
	const std::string ThreadName = ss.str();

	RLog("[Thread: %s][%d] ", ThreadName.c_str(), GetTimeInMillisecond());
}

#if 0
#define RLogThread(...)			{ DisplayThreadAndTime(); RLog(__VA_ARGS__); }
#else
#define RLogThread(...)
#endif

//////////////////////////////////////////////////////////////////////////
// Log thread - End
//////////////////////////////////////////////////////////////////////////

void ThreadWorker_Render(int begin, int end, int MaxBounceCount, const RenderOption& InOption = RenderOption())
{
	const RVec3 ViewPoint(0, 0, 7.0f);
	const RayTracerScene* Scene = RayTracerProgram::GetActiveInstance().GetScene();
	const float Aspect = (float)bitmapWidth / (float)bitmapHeight;

	for (int PixelIndex = begin; PixelIndex <= end; PixelIndex++)
	{
		int x, y;
		BufferIndexToCoord(PixelIndex, x, y);
		float dx = -(float)(x - bitmapWidth / 2) / (bitmapWidth * 2) * Aspect;
		float dy = -(float)(y - bitmapHeight / 2) / (bitmapHeight * 2);

		RVec3 c = RVec3::Zero();

#if ENABLE_ANTIALIASING
		static const float inv_pixel_radius = 1.0f / (bitmapWidth * 4);

		static const float ox[4] = { 0.0f, inv_pixel_radius, 0.0f, inv_pixel_radius };
		static const float oy[4] = { 0.0f, 0.0f, inv_pixel_radius, inv_pixel_radius };

		static const float offset_radius = inv_pixel_radius * 0.5f;

		// Randomly sample 2x2 nearby pixels for antialiasing
		for (int i = 0; i < 4; i++)
		{
			float offset_x = ox[i];
			float offset_y = oy[i];

			// Randomize sampling point
			offset_x += (RMath::Random() - 0.5f) * offset_radius;
			offset_y += (RMath::Random() - 0.5f) * offset_radius;

			RVec3 Dir(dx + offset_x, dy + offset_y, -0.5f);
			RRay ray(ViewPoint, Dir.GetNormalizedVec3(), 1000.0f);
			c += Scene->RayTrace(ray, MaxBounceCount, InOption);
		}

		c /= 4.0f;
#else
		RVec3 Dir(dx, dy, 0.5f);
		RRay ray(ViewPoint, Dir.GetNormalizedVec3(), 1000.0f);
		c = Scene->RayTrace(ray, MaxBounceCount, InOption);
#endif  // ENABLE_ANTIALIASING

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
	std::thread::id this_id = std::this_thread::get_id();
	std::stringstream ss;
	ss << this_id;
	const std::string ThreadName = ss.str();

	RLogThread("Start worker thread [%s]\n", ThreadName.c_str());

	RenderThreadTaskQueue& TaskQueue = RenderThreadTaskQueue::Get();

	while (1)
	{
		RenderThreadTask Task;
		{
			std::unique_lock<std::mutex> ThreadLock(TaskQueue.GetMutex());

			RLogThread("Thread [%s] is waiting\n", ThreadName.c_str());

			// If the task queue is empty, wait until a new task is queued.
			TaskQueue.GetWorkerThreadCondition().wait(ThreadLock, [] {
				return RenderThreadTaskQueue::Get().GetNumTasks() > 0 || RayTracerProgram::GetActiveInstance().IsTerminating();
			});

			// Mutex is locked now

			// Handle program terminating
			if (RayTracerProgram::GetActiveInstance().IsTerminating())
			{
				RLogThread("Terminating thread [%s]\n", ThreadName.c_str());
				return;
			}

			// Get a task from task queue
			TaskQueue.PopTask(&Task);

			RLogThread("Remaining tasks in queue: %d\n", TaskQueue.GetNumTasks());

			// Mutex will be unlocked when leaving the scope. Other worker threads will then get tasks afterwards.
		}

		// The max times ray can bounce between surfaces
		static const int MaxBounceTimes = 10;

		RLogThread("Executing render task on thread [%s]...\n", ThreadName.c_str());
		ThreadWorker_Render(Task.Start, Task.End, MaxBounceTimes, Task.Option);
		RLogThread("Render task is done on thread [%s]!\n", ThreadName.c_str());

		TaskQueue.NotifySingleTaskDone();
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
	const int ThreadCount = ThreadUtils::DetectWorkerThreadsNum();

	RLog("Starting rendering tasks on %d threads...\n", ThreadCount);
	ScopeAutoJoinedThreads WorkerThreads;
	RenderThreadTaskQueue& TaskQueue = RenderThreadTaskQueue::Get();

	RenderOption BaseColorOption;
	BaseColorOption.UseBaseColor = true;
	const int MaxBufferIdx = bitmapHeight * bitmapWidth - 1;
	const int NumTaskRows = 10;

	// Start all worker threads
	for (int i = 0; i < ThreadCount; i++)
	{
		std::thread Worker(ThreadTaskWorker);
		WorkerThreads.AddThread(Worker);
	}

	// Draw base color for preview
	{
		// Split rendering area to tasks
		for (int i = 0; i < bitmapHeight; i += NumTaskRows)
		{
			//RLog("Creating task [%d/%d]...\n", i + 1, bitmapHeight);

			int Start = i * bitmapWidth;
			int End = Math::Min((i + NumTaskRows) * bitmapWidth - 1, MaxBufferIdx);

			TaskQueue.PushTask(RenderThreadTask(Start, End, BaseColorOption));
		}

		RLog("Done pushing all tasks.\n");

		// Wait until all threads finish their work of current sample
		TaskQueue.WaitForAllTasksDone();

#if 0
		return;
#endif
	}

	auto StartTime = std::chrono::system_clock::now();
	auto LastFrameTime = StartTime;

	for (int Sample = 0; Sample < TotalSamplesNum; Sample++)
	{
		// Split rendering area to tasks
		for (int i = 0; i < bitmapHeight; i += NumTaskRows)
		{
			// One task covers rendering a single line from left to right
			int Start = i * bitmapWidth;
			int End = Math::Min((i + NumTaskRows) * bitmapWidth - 1, MaxBufferIdx);

			TaskQueue.PushTask(RenderThreadTask(Start, End, RenderOption()));
		}

		// Wait until all threads finish their work of current sample
		TaskQueue.WaitForAllTasksDone();

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
		RayTracerProgram& ActiveProgram = RayTracerProgram::GetActiveInstance();
		if (ActiveProgram.IsTerminating())
		{
			break;
		}

		ActiveProgram.GetRenderWindow()->SetTitle(TextBuffer);
	}
    
    // Save result image to file
    {
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];
        
        time (&rawtime);
        timeinfo = localtime(&rawtime);
        
        strftime(buffer,sizeof(buffer),"%Y-%m-%d_%H-%M-%S", timeinfo);
        std::string Filename("Output_");
        Filename += buffer;
        Filename += ".png";
        
        RTexture::SaveBufferToPNG(Filename.c_str(), bitcolor, bitmapWidth, bitmapHeight);
        RLog("Image saved as %s\n", Filename.c_str());
    }
}

RayTracerProgram::RayTracerProgram()
	: bQuit(false)
{
	assert(CurrentInstance == nullptr);
	CurrentInstance = this;
}

RayTracerProgram::~RayTracerProgram()
{
	assert(CurrentInstance != nullptr);
	CurrentInstance = nullptr;
}

void RayTracerProgram::Run()
{
	srand((unsigned int)time(nullptr));

	RLog("Initializing pseudo random numbers... ");
	RMath::InitPseudoRandomUnitVector();
	RLog("Done\n");

	MainRenderWindow.Create(bitmapWidth, bitmapHeight);
	MainRenderWindow.SetRenderBufferParameters(bitmapWidth, bitmapHeight, bitcolor);

	SetupScene();

	// Begin ray tracing render thread
	RayTracerMainThread = std::thread(UpdateBitmapPixels);

	MainRenderWindow.RunWindowLoop(this);
}

void RayTracerProgram::ExecuteCleanup()
{
	bQuit = true;
	RenderThreadTaskQueue::Get().NotifyQuit();
	RayTracerMainThread.join();

	MainRenderWindow.Destroy();
}

void RayTracerProgram::SetupScene()
{
	Scene.AddShape(RSphere::Create(RVec3(0.0f, 2.3f, -2.0f), 0.9f),
		MakeUnique<SurfaceMaterial_Blend>(
			MakeUnique<SurfaceMaterial_Reflective>(),
			MakeUnique<SurfaceMaterial_Diffuse>(RVec3(1.0f, 0.5f, 0.1f)),
			0.5f)
	);

	Scene.AddShape(RSphere::Create(RVec3(-1.5f, 2.2f, -3.0f), 0.5f),
		MakeUnique<SurfaceMaterial_Diffuse>(RVec3(0.1f, 1.0f, 0.2f))
	);

	Scene.AddShape(RSphere::Create(RVec3(-0.2f, -1.8f, -1.0f), 0.5f),
		MakeUnique<SurfaceMaterial_Blend>(
			MakeUnique<SurfaceMaterial_Reflective>(),
			MakeUnique<SurfaceMaterial_Diffuse>(RVec3(0.5f, 0.0f, 0.2f)),
			0.5f)
	);

	Scene.AddShape(RSphere::Create(RVec3(2.8f, -1.2f, -4.0f), 1.5f),
		MakeUnique<SurfaceMaterial_Combine>(
			MakeUnique<SurfaceMaterial_Blend>(
				MakeUnique<SurfaceMaterial_Reflective>(RVec3(0.95f, 0.75f, 0.1f)),
				MakeUnique<SurfaceMaterial_Diffuse>(RVec3(0.95f, 0.75f, 0.1f)),
				0.5f),
			MakeUnique<SurfaceMaterial_Emissive>(RVec3(0.95f, 0.75f, 0.1f))
		)
	);

	//// Ceiling light
	//Scene.AddShape(RSphere::Create(RVec3(0.0f, 5.0f, 0.0f), 0.5f),
	//	MakeUnique<SurfaceMaterial_Emissive>(RVec3(5.0f, 2.0f, 6.0f))
	//);

	Scene.AddShape(RCapsule::Create(RVec3(-1.5f, -0.5f, 0.0f), RVec3(-2.0f, -1.5f, 0.0f), 0.5f),
		MakeUnique<SurfaceMaterial_Combine>(
			MakeUnique<SurfaceMaterial_Blend>(
				MakeUnique<SurfaceMaterial_Reflective>(RVec3(0.25f, 0.75f, 0.6f)),
				MakeUnique<SurfaceMaterial_Diffuse>(RVec3(0.25f, 0.75f, 0.6f)),
				0.5f),
			MakeUnique<SurfaceMaterial_Emissive>(RVec3(0.25f, 0.75f, 0.6f))
		)
	);

	// Walls
	{
		// Ground
		Scene.AddShape(RPlane::Create(RVec3(0.0f, 1.0f, 0.0f), RVec3(0.0f, -2.0f, 0.0f)),
			MakeUnique<SurfaceMaterial_Blend>(
				MakeUnique<SurfaceMaterial_Reflective>(RVec3(1, 1, 1), 0.1f),
				MakeUnique<SurfaceMaterial_DiffuseChecker>(),
				0.5f)
		);

		//// Ceiling / Sky light plane
		//Scene.AddShape(RPlane::Create(RVec3(0.0f, -1.0f, 0.0f), RVec3(0.0f, 5.0f, 0.0f)),
		//	MakeUnique<SurfaceMaterial_Diffuse>(RVec3(1.2f, 1.2f, 1.5f))
		//);

		//// Back wall
		//Scene.AddShape(RPlane::Create(RVec3(0.0f, 0.0f, 1.0f), RVec3(0.0f, 0.0f, -5.0f)),
		//	MakeUnique<SurfaceMaterial_DiffuseChecker>()
		//);

		//// Front wall (behind the camera)
		//Scene.AddShape(RPlane::Create(RVec3(0.0f, 0.0f, -1.0f), RVec3(0.0f, 0.0f, 10.0f)),
		//	MakeUnique<SurfaceMaterial_DiffuseChecker>()
		//);

		//// Right wall
		//Scene.AddShape(RPlane::Create(RVec3(1.0f, 0.0f, 0.0f), RVec3(-5.0f, 0.0f, 0.0f)),
		//	MakeUnique<SurfaceMaterial_DiffuseChecker>()
		//);

		//// Left wall
		//Scene.AddShape(RPlane::Create(RVec3(-1.0f, 0.0f, 0.0f), RVec3(5.0f, 0.0f, 0.0f)),
		//	MakeUnique<SurfaceMaterial_DiffuseChecker>()
		//);
	}

	// Meshes
	Scene.AddShape(RMeshShape::Create("../Data/unitychan.obj"),
		MakeUnique<SurfaceMaterial_Blend>(
			MakeUnique<SurfaceMaterial_Reflective>(RVec3(1, 1, 1), 0.2f),
			MakeUnique<SurfaceMaterial_Diffuse>(RVec3(1.0f, 1.0f, 1.0f)),
			1.0f)
	);
}
