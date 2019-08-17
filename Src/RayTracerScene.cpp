//=============================================================================
// RayTracerScene.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "RayTracerScene.h"
#include "Math.h"
#include "Platform.h"
#include "RayTracerProgram.h"

#define USE_LIGHTS 0

LightData GSceneLights[] =
{
	//{ LT_Directional,	RVec3(0.0f, -1.0f, 0.0f), RVec3(1, 1, 1) },
	{ LT_Point,			RVec3(0.0f, -4.5f, 0.0f), RVec3(1, 1, 1) },
};

RayTracerScene::RayTracerScene()
{

}

void RayTracerScene::AddShape(unique_ptr<RShape> Shape, RMaterial Material /*= RMaterial()*/)
{
	Shape->SetMaterial(Material);
	SceneShapes.push_back(std::move(Shape));
}

void RayTracerScene::AddShape(unique_ptr<RShape> Shape, unique_ptr<ISurfaceMaterial> SurfaceMaterial)
{
	Shape->SetSurfaceMaterial(std::move(SurfaceMaterial));
	SceneShapes.push_back(std::move(Shape));
}

RVec3 RayTracerScene::RayTrace(const RRay& InRay, int MaxBounceTimes /*= 10*/, const RenderOption& InOption /*= RenderOption()*/) const
{
	// Stop the recursion function when program is exiting
	if (RayTracerProgram::GetActiveInstance().IsTerminating())
	{
		return RVec3(0, 0, 0);
	}

	if (MaxBounceTimes == 0)
	{
		return RVec3::Zero();
	}

	RVec3 FinalColor = RVec3::Zero();

	RayHitResult Result;
	int HitShapeIndex = FindIntersectionWithScene(InRay, Result);

	if (HitShapeIndex != -1)
	{
		auto& HitShape = SceneShapes[HitShapeIndex];
		auto& Material = HitShape->GetMaterial();
		ISurfaceMaterial* const SurfaceMaterial = HitShape->GetSurfaceMaterial();

		RVec3 SurfaceColor = Material.Color;

		// Make checkerboard pattern
		if (Material.UseCheckerboardPattern)
		{
			bool color = false;
			float fx = Result.HitPosition.x * 0.2f;
			float fy = Result.HitPosition.y * 0.2f;
			float fz = Result.HitPosition.z * 0.2f;

			if (fx - floorf(fx) > 0.5f)
				color = !color;
			if (fz - floorf(fz) > 0.5f)
				color = !color;
			if (fy - floorf(fy) > 0.5f)
				color = !color;

			if (!color)
				SurfaceColor *= 0.5f;
		}

		if (InOption.UseBaseColor)
		{
			// Base color render pass for previewing
			if (Material.MaterialFlags & MT_Diffuse)
			{
				FinalColor += SurfaceColor;
				FinalColor *= RVec3::Dot(Result.HitNormal, RVec3(0, 1, 0)) * 0.5f + 0.5f;
			}

			if (Material.MaterialFlags & MT_Emissive)
			{
				FinalColor += SurfaceColor;
			}

			if (SurfaceMaterial)
			{
				FinalColor += SurfaceMaterial->PreviewColor(Result);
			}
		}
		else
		{
			float DiffuseRatio = 1.0f;

			// The remaining distance ray will travel
			float RayDistance = InRay.Distance - Result.Distance;

			if (RayDistance > 0)
			{
				if (Material.MaterialFlags & MT_Reflective)
				{
					const float ReflectiveRatio = 0.5f;
					DiffuseRatio = 1.0f - ReflectiveRatio;

					// Ignore hitting a surface from its back side
					if (RVec3::Dot(InRay.Direction, Result.HitNormal) < 0)
					{
						// The direction of reflection
						RVec3 newDir = InRay.Direction.Reflect(Result.HitNormal);
						RRay reflRay(Result.HitPosition + newDir * 0.001f, newDir, RayDistance);

						FinalColor += RayTrace(reflRay, MaxBounceTimes - 1, InOption) * ReflectiveRatio;
					}
				}

				if (Material.MaterialFlags & MT_Diffuse)
				{
					float DotProductResult = 1.0f;
					RVec3 DiffuseColor = RVec3(1.0f, 1.0f, 1.0f);

					// Ray bounces off a surface in a random direction of a hemisphere
					RVec3 DiffuseReflectionDirection = RMath::RandomHemisphereDirection(Result.HitNormal);
					RRay DiffuseRay(Result.HitPosition + DiffuseReflectionDirection * 0.001f, DiffuseReflectionDirection, RayDistance);

					// Lambertian reflectance
					DotProductResult = Math::Max(0.0f, RVec3::Dot(Result.HitNormal, DiffuseReflectionDirection));

					DiffuseColor = RayTrace(DiffuseRay, MaxBounceTimes - 1, InOption);

#if (USE_LIGHTS == 1)
					// Apply diffuse lighting
					for (int i = 0; i < ARRAYSIZE(GSceneLights); i++)
					{
						LightData* l = &GSceneLights[i];
						FinalColor += CalculateLightColor(l, Result, SurfaceColor) * DiffuseRatio;
					}
#endif

					FinalColor += SurfaceColor * DiffuseColor * DotProductResult * DiffuseRatio;
				}
			}

			if (Material.MaterialFlags & MT_Emissive)
			{
				FinalColor += SurfaceColor;
			}

			if (SurfaceMaterial)
			{
				RRay OutRay;
				RVec3 SurfaceReflection = SurfaceMaterial->BounceViewRay(InRay, Result, OutRay);
				FinalColor += SurfaceReflection * RayTrace(OutRay, MaxBounceTimes - 1, InOption);
			}
		}
	}
    else
    {
        // Did not hit any shapes, returns sky color
        return RVec3(0.4f, 0.6f, 0.9f);
    }

	return FinalColor;
}

int RayTracerScene::FindIntersectionWithScene(RRay TestRay, RayHitResult& OutResult) const
{
	int HitShapeIndex = -1;
	int Index = 0;

	// Get nearest hit point for this ray
	for (auto& Shape : SceneShapes)
	{
		// If shape has a bound, run bound intersection test for early out.
		// Note: Shapes such as planes don't have bounds. Always run a full intersection test on them.
		if (!Shape->HasCullingBounds() || TestRay.TestIntersectionWithAabb(Shape->GetBounds()))
		{
			bool hit = Shape->TestRayIntersection(TestRay, &OutResult);

			if (hit)
			{
				// Shorten distance of current testing ray
				TestRay.Distance = OutResult.Distance;
				HitShapeIndex = Index;
			}
		}

		Index++;
	}

	return HitShapeIndex;
}

RVec3 RayTracerScene::CalculateLightColor(const LightData* InLight, const RayHitResult &InHitResult, const RVec3& InSurfaceColor) const
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
	for (auto& Shape : SceneShapes)
	{
		if (!Shape->HasCullingBounds() || ShadowRay.TestIntersectionWithAabb(Shape->GetBounds()))
		{
			bool hit = Shape->TestRayIntersection(ShadowRay);

			if (hit)
			{
				IsInShadow = true;
				break;
			}
		}
	}

	if (IsInShadow)
	{
		// Surface is in shadow, no light contribution
		return RVec3::Zero();
	}
	else
	{
		float ldp = Math::Max(0.0f, RVec3::Dot(InHitResult.HitNormal, LightDirection));
		return InSurfaceColor * ldp;
	}
}