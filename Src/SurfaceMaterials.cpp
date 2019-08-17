//=============================================================================
// SurfaceMaterials.cpp by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#include "SurfaceMaterials.h"

#include "Math.h"

using namespace std;

SurfaceMaterial_Diffuse::SurfaceMaterial_Diffuse(const RVec3 InAlbedo /*= RVec3(1.0f, 1.0f, 1.0f)*/)
	: Albedo(InAlbedo)
{
}

RVec3 SurfaceMaterial_Diffuse::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	// The remaining distance ray will travel
	float RayDistance = InViewRay.Distance - HitResult.Distance;

	// Ray bounces off a surface in a random direction of a hemisphere
	RVec3 DiffuseReflectionDirection = RMath::RandomHemisphereDirection(HitResult.HitNormal);
	OutViewRay = RRay(HitResult.HitPosition + DiffuseReflectionDirection * 0.001f, DiffuseReflectionDirection, RayDistance);

	// Lambertian reflectance
	float DotProductResult = Math::Max(0.0f, RVec3::Dot(HitResult.HitNormal, DiffuseReflectionDirection));

	return Albedo * DotProductResult;
}

RVec3 SurfaceMaterial_Diffuse::PreviewColor(const RayHitResult& HitResult) const
{
	return Albedo * (RVec3::Dot(HitResult.HitNormal, RVec3(0, 1, 0)) * 0.5f + 0.5f);
}

SurfaceMaterial_DiffuseChecker::SurfaceMaterial_DiffuseChecker(const RVec3 InAlbedo /*= RVec3(1.0f, 1.0f, 1.0f)*/)
	: SurfaceMaterial_Diffuse(InAlbedo)
{
}

RVec3 SurfaceMaterial_DiffuseChecker::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	float Factor = IsBrighterArea(HitResult.HitPosition) ? 1.0f : 0.5f;
	return SurfaceMaterial_Diffuse::BounceViewRay(InViewRay, HitResult, OutViewRay) * Factor;
}

RVec3 SurfaceMaterial_DiffuseChecker::PreviewColor(const RayHitResult& HitResult) const
{
	float Factor = IsBrighterArea(HitResult.HitPosition) ? 1.0f : 0.5f;
	return SurfaceMaterial_Diffuse::PreviewColor(HitResult) * Factor;
}

bool SurfaceMaterial_DiffuseChecker::IsBrighterArea(const RVec3& WorldPosition) const
{
	bool bResult = false;
	float fx = WorldPosition.x * 0.2f;
	float fy = WorldPosition.y * 0.2f;
	float fz = WorldPosition.z * 0.2f;

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

RVec3 SurfaceMaterial_Reflective::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
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

	OutViewRay = RRay(HitResult.HitPosition + newDir * 0.001f, newDir, RayDistance);

	return Albedo;
}

RVec3 SurfaceMaterial_Reflective::PreviewColor(const RayHitResult& HitResult) const
{
	return Albedo;
}

SurfaceMaterial_Blend::SurfaceMaterial_Blend(unique_ptr<ISurfaceMaterial> InMaterialA, unique_ptr<ISurfaceMaterial> InMaterialB, float InBlendFactor)
	: BlendMaterialA(std::move(InMaterialA))
	, BlendMaterialB(std::move(InMaterialB))
	, BlendFactor(RMath::Clamp(InBlendFactor, 0.0f, 1.0f))
{

}

RVec3 SurfaceMaterial_Blend::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	return RMath::Random() > BlendFactor ? BlendMaterialA->BounceViewRay(InViewRay, HitResult, OutViewRay) : BlendMaterialB->BounceViewRay(InViewRay, HitResult, OutViewRay);
}

RVec3 SurfaceMaterial_Blend::PreviewColor(const RayHitResult& HitResult) const
{
	return RMath::Random() > BlendFactor ? BlendMaterialA->PreviewColor(HitResult) : BlendMaterialB->PreviewColor(HitResult);
}

RVec3 SurfaceMaterial_Null::BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const
{
	// The remaining distance ray will travel
	float RayDistance = InViewRay.Distance - HitResult.Distance;

	OutViewRay = RRay(HitResult.HitPosition + InViewRay.Direction * 0.001f, InViewRay.Direction, RayDistance);

	return RVec3(1, 1, 1);
}

RVec3 SurfaceMaterial_Null::PreviewColor(const RayHitResult& HitResult) const
{
	return RVec3(0, 0, 0);
}
