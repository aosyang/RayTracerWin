//=============================================================================
// RayTracerProgram.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Platform.h"

#if (PLATFORM_OSX)
#include "OSX/OSXWindow.h"
#elif (PLATFORM_WIN32)
#include "Windows/RenderWindow.h"
#endif

#include "RayTracerScene.h"

#include <thread>
#include <assert.h>


class RayTracerProgram
{
public:
	RayTracerProgram();
	~RayTracerProgram();

	// Get the active instance of program
	static RayTracerProgram& GetActiveInstance();

	void Run();

	// Execute final cleanup before exiting the program
	void ExecuteCleanup();

	// Get the main render window of program
	RenderWindow* GetRenderWindow();

	// Get the scene of program
	RayTracerScene* GetScene();

	// Has program requested to quit
	bool IsTerminating() const;

protected:
	void SetupScene();

private:
	// Active instance of program
	static RayTracerProgram* CurrentInstance;

	RenderWindow MainRenderWindow;

	RayTracerScene Scene;

	std::thread RayTracerMainThread;

	bool bQuit;
};


FORCEINLINE RayTracerProgram& RayTracerProgram::GetActiveInstance()
{
	assert(CurrentInstance);
	return *CurrentInstance;
}

FORCEINLINE RenderWindow* RayTracerProgram::GetRenderWindow()
{
	return &MainRenderWindow;
}

FORCEINLINE RayTracerScene* RayTracerProgram::GetScene()
{
	return &Scene;
}

FORCEINLINE bool RayTracerProgram::IsTerminating() const
{
	return bQuit;
}
