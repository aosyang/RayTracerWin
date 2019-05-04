//=============================================================================
// RayTracerScene.h by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "Shapes.h"
#include "Light.h"

#include <vector>
#include <memory>
#include <atomic>

using std::unique_ptr;

// Surface material types
enum EMaterial
{
	MT_Emissive		= 1 << 0,
	MT_Diffuse		= 1 << 1,
	MT_Reflective	= 1 << 2,
};

struct RenderOption
{
	// Do not ray trace. Use geometry color for a fast preview pass.
	bool UseBaseColor;

	RenderOption()
		: UseBaseColor(false)
	{}
};

// Data structure of a shape in the scene
struct ShapeData
{
	RShape*	Shape;
};

class RayTracerScene
{
public:
	RayTracerScene();

	// Add a shape to scene
	void AddShape(unique_ptr<RShape> Shape, RMaterial Material = RMaterial());

	// Notify about exiting the program, all function should stop
	void NotifyTerminatingProgram();

	// Check if program is about to exit
	bool IsTerminatingProgram() const;

	// Run the ray tracing along a ray and get the color
	RVec3 RayTrace(const RRay& InRay, int MaxBounceTimes = 10, const RenderOption& InOption = RenderOption()) const;

	// Test a ray against the scene and find intersection result
	int FindIntersectionWithScene(RRay TestRay, RayHitResult& OutResult) const;

protected:
	RVec3 CalculateLightColor(const LightData* InLight, const RayHitResult &InHitResult, const RVec3& InSurfaceColor) const;

private:
	std::vector<unique_ptr<RShape>> SceneShapes;

	// State of program exiting
	std::atomic<bool> bExiting;
};
