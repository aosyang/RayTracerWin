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

void RayTracerScene::AddShape(unique_ptr<RShape> Shape, unique_ptr<ISurfaceMaterial> SurfaceMaterial)
{
	Shape->SetSurfaceMaterial(std::move(SurfaceMaterial));
	SceneShapes.push_back(std::move(Shape));
}

RVec3 RayTracerScene::RayTrace(const RRay& InRay, int MaxBounceTimes, const RenderOption& InOption /*= RenderOption()*/) const
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
		ISurfaceMaterial* const SurfaceMaterial = HitShape->GetSurfaceMaterial();

		if (InOption.UseBaseColor)
		{
			// Base color render pass for previewing
			if (SurfaceMaterial)
			{
				FinalColor += SurfaceMaterial->PreviewColor(Result) * Result.SampledColor;
			}
		}
		else
		{
			if (SurfaceMaterial)
			{
				RRay OutRay;
				auto BounceResult = SurfaceMaterial->BounceViewRay(InRay, Result, OutRay);

				if (RMath::Random() <= Result.SampledAlpha)
				{
					// Early out further ray tracing if attenuation reaches zero
					if (BounceResult.Attenuation.IsNonZero())
					{
						FinalColor += BounceResult.Attenuation * RayTrace(OutRay, MaxBounceTimes - 1, InOption) * Result.SampledColor;
					}

					FinalColor += BounceResult.Emissive;
				}
				else
				{
					// Transparent, continue tracing the view ray in current direction
					float RayDistance = InRay.Distance - Result.Distance;
					OutRay = RRay(Result.HitPosition + InRay.Direction * BounceRayStartOffset, InRay.Direction, RayDistance);
					FinalColor += RayTrace(OutRay, MaxBounceTimes - 1, InOption);
				}
			}
		}
	}
    else
    {
        // Did not hit any shapes, returns sky color
        float t = 0.5f * (InRay.Direction.y + 1.0f);
        return (1.0f - t) * RVec3(1.0f, 1.0f, 1.0f) + t * RVec3(0.5f, 0.7f, 1.0f);
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

	RRay ShadowRay(InHitResult.HitPosition + LightDirection * BounceRayStartOffset, LightDirection, dist);
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