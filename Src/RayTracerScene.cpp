//=============================================================================
// RayTracerScene.cpp by Shiyang Ao, 2018 All Rights Reserved.
//
// 
//=============================================================================

#include "RayTracerScene.h"
#include "Math.h"
#include "Platform.h"

#define USE_LIGHTS 0

LightData GSceneLights[] =
{
	//{ LT_Directional,	RVec3(0.0f, -1.0f, 0.0f), RVec3(1, 1, 1) },
	{ LT_Point,			RVec3(0.0f, -4.5f, 0.0f), RVec3(1, 1, 1) },
};

RayTracerScene::RayTracerScene()
	: bExiting(false)
{

}

void RayTracerScene::AddShape(unique_ptr<RShape> Shape, RMaterial Material /*= RMaterial()*/)
{
	Shape->SetMaterial(Material);
	SceneShapes.push_back(std::move(Shape));
}

void RayTracerScene::NotifyTerminatingProgram()
{
	bExiting = true;
}

bool RayTracerScene::IsTerminatingProgram() const
{
	return bExiting;
}

RVec3 RayTracerScene::RayTrace(const RRay& InRay, int MaxBounceTimes /*= 10*/, const RenderOption& InOption /*= RenderOption()*/) const
{
	// Stop the recursion function when program is exiting
	if (bExiting)
	{
		return RVec3(0, 0, 0);
	}

	RRay TestRay = InRay;

	if (MaxBounceTimes == 0)
		return RVec3::Zero();

	RVec3 FinalColor = RVec3::Zero();

	RayHitResult result;
	int HitShapeIndex = -1;

	int Index = 0;

	// Get nearest hit point for this ray
	for (auto& Shape : SceneShapes)
	{
		if (!Shape->HasCullingBounds() || TestRay.TestIntersectionWithAabb(Shape->GetBounds()))
		{
			bool hit = Shape->TestRayIntersection(TestRay, &result);

			if (hit)
			{
				// Shorten distance of current testing ray
				TestRay.Distance = result.Distance;
				HitShapeIndex = Index;
			}
		}

		Index++;
	}

	if (HitShapeIndex != -1)
	{
		auto& HitShape = SceneShapes[HitShapeIndex];
		auto& Material = HitShape->GetMaterial();

		float DiffuseRatio = 1.0f;

		if (Material.MaterialFlags & MT_Reflective)
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

		RVec3 SurfaceColor = Material.Color;

		// Make checkerboard pattern
		if (Material.UseCheckerboardPattern)
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

		if (Material.MaterialFlags & MT_Diffuse)
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

				if (InOption.UseBaseColor)
				{
					FinalColor *= RVec3::Dot(result.HitNormal, RVec3(0, -1, 0)) * 0.5f + 0.5f;
				}
			}
		}

		if (Material.MaterialFlags & MT_Emissive)
		{
			FinalColor += SurfaceColor;
		}
	}

	return FinalColor;
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
		float ldp = Math::Max(0.0f, InHitResult.HitNormal.Dot(LightDirection));
		return InSurfaceColor * ldp;
	}
}