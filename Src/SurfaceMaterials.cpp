//=============================================================================
// SurfaceMaterials.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "SurfaceMaterials.h"

#include "Math.h"

using namespace std;

float BounceRayStartOffset = 0.0001f;

SurfaceMaterial_Diffuse::SurfaceMaterial_Diffuse(const RVec3 InAlbedo /*= RVec3(1.0f, 1.0f, 1.0f)*/)
	: Albedo(InAlbedo)
{
}

ViewRayBounceResult SurfaceMaterial_Diffuse::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	// The remaining distance ray will travel
	float RayDistance = InViewRay.Distance - HitResult.Distance;

	// Ray bounces off a surface in a random direction of a hemisphere
	RVec3 DiffuseReflectionDirection = RMath::RandomHemisphereDirection(HitResult.HitNormal);
	OutViewRay = RRay(HitResult.HitPosition + DiffuseReflectionDirection * BounceRayStartOffset, DiffuseReflectionDirection, RayDistance);

	// Lambertian reflectance
	float DotProductResult = Math::Max(0.0f, RVec3::Dot(HitResult.HitNormal, DiffuseReflectionDirection));

	return ViewRayBounceResult(Albedo * DotProductResult);
}

RVec3 SurfaceMaterial_Diffuse::PreviewColor(const RayHitResult& HitResult) const
{
	return Albedo * (RVec3::Dot(HitResult.HitNormal, RVec3(0, 1, 0)) * 0.5f + 0.5f);
}

SurfaceMaterial_DiffuseChecker::SurfaceMaterial_DiffuseChecker(const RVec3 InAlbedo /*= RVec3(1.0f, 1.0f, 1.0f)*/, float InPatternSize /*= 5.0f*/)
	: SurfaceMaterial_Diffuse(InAlbedo)
{
    if (FLT_EQUAL_ZERO(InPatternSize))
    {
        ReciprocalPatternSize = 1.0f;
    }
    else
    {
        ReciprocalPatternSize = 1.0f / InPatternSize;
    }
}

ViewRayBounceResult SurfaceMaterial_DiffuseChecker::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	float Factor = IsBrighterArea(HitResult.HitPosition) ? 1.0f : 0.5f;
	auto Result = SurfaceMaterial_Diffuse::BounceViewRay(InViewRay, HitResult, OutViewRay);
	Result.Attenuation *= Factor;
	return Result;
}

RVec3 SurfaceMaterial_DiffuseChecker::PreviewColor(const RayHitResult& HitResult) const
{
	float Factor = IsBrighterArea(HitResult.HitPosition) ? 1.0f : 0.5f;
	return SurfaceMaterial_Diffuse::PreviewColor(HitResult) * Factor;
}

bool SurfaceMaterial_DiffuseChecker::IsBrighterArea(const RVec3& WorldPosition) const
{
	bool bResult = false;
	float fx = WorldPosition.x * ReciprocalPatternSize;
	float fy = WorldPosition.y * ReciprocalPatternSize;
	float fz = WorldPosition.z * ReciprocalPatternSize;

	if (fx - floorf(fx) > 0.5f)
	{
		bResult = !bResult;
	}

	if (fz - floorf(fz) > 0.5f)
	{
		bResult = !bResult;
	}

	if (fy - floorf(fy) > 0.5f)
	{
		bResult = !bResult;
	}

	return bResult;
}

SurfaceMaterial_Reflective::SurfaceMaterial_Reflective(const RVec3 InAlbedo /*= RVec3(1.0f, 1.0f, 1.0f)*/, float InFuzziness /*= 0.0f*/)
	: Albedo(InAlbedo)
	, Fuzziness(InFuzziness)
{
}

ViewRayBounceResult SurfaceMaterial_Reflective::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	//if (RVec3::Dot(InViewRay.Direction, HitResult.HitNormal) >= 0)
	//{
	//	return RVec3(0, 0, 0);
	//}

	// The remaining distance ray will travel
	float RayDistance = InViewRay.Distance - HitResult.Distance;

	// The direction of reflection
	RVec3 newDir = InViewRay.Direction.Reflect(HitResult.HitNormal);

	if (Fuzziness > 0.0f)
	{
		newDir += RMath::RandomUnitVector() * Fuzziness;
		newDir.Normalize();
	}

	OutViewRay = RRay(HitResult.HitPosition + newDir * BounceRayStartOffset, newDir, RayDistance);

	return ViewRayBounceResult(Albedo);
}

RVec3 SurfaceMaterial_Reflective::PreviewColor(const RayHitResult& HitResult) const
{
	return Albedo;
}

SurfaceMaterial_Emissive::SurfaceMaterial_Emissive(const RVec3 InColor)
	: Color(InColor)
{
}

ViewRayBounceResult SurfaceMaterial_Emissive::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	OutViewRay = InViewRay;
	
	// Use 0 attenuation so view ray will trace no further
	return ViewRayBounceResult(RVec3(0, 0, 0), Color);
}

RVec3 SurfaceMaterial_Emissive::PreviewColor(const RayHitResult& HitResult) const
{
	return Color;
}

SurfaceMaterial_Blend::SurfaceMaterial_Blend(unique_ptr<ISurfaceMaterial> InMaterialA, unique_ptr<ISurfaceMaterial> InMaterialB, float InBlendFactor)
	: BlendMaterialA(std::move(InMaterialA))
	, BlendMaterialB(std::move(InMaterialB))
	, BlendFactor(RMath::Clamp(InBlendFactor, 0.0f, 1.0f))
{

}

ViewRayBounceResult SurfaceMaterial_Blend::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	return RMath::Random() > BlendFactor ? BlendMaterialA->BounceViewRay(InViewRay, HitResult, OutViewRay) : BlendMaterialB->BounceViewRay(InViewRay, HitResult, OutViewRay);
}

RVec3 SurfaceMaterial_Blend::PreviewColor(const RayHitResult& HitResult) const
{
	return RMath::Random() > BlendFactor ? BlendMaterialA->PreviewColor(HitResult) : BlendMaterialB->PreviewColor(HitResult);
}

SurfaceMaterial_Combine::SurfaceMaterial_Combine(std::unique_ptr<ISurfaceMaterial> InMaterialA, std::unique_ptr<ISurfaceMaterial> InMaterialB)
	: MaterialA(std::move(InMaterialA))
	, MaterialB(std::move(InMaterialB))
{
}

ViewRayBounceResult SurfaceMaterial_Combine::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	return MaterialA->BounceViewRay(InViewRay, HitResult, OutViewRay) + MaterialB->BounceViewRay(InViewRay, HitResult, OutViewRay);
}

RVec3 SurfaceMaterial_Combine::PreviewColor(const RayHitResult& HitResult) const
{
	return MaterialA->PreviewColor(HitResult) + MaterialB->PreviewColor(HitResult);
}

ViewRayBounceResult SurfaceMaterial_Null::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	// The remaining distance ray will travel
	float RayDistance = InViewRay.Distance - HitResult.Distance;

	OutViewRay = RRay(HitResult.HitPosition + InViewRay.Direction * BounceRayStartOffset, InViewRay.Direction, RayDistance);

	return ViewRayBounceResult(RVec3(1, 1, 1));
}

RVec3 SurfaceMaterial_Null::PreviewColor(const RayHitResult& HitResult) const
{
	return RVec3(0, 0, 0);
}
