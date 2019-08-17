//=============================================================================
// SurfaceMaterials.h by Shiyang Ao, 2019 All Rights Reserved.
//
// 
//=============================================================================

#pragma once

#include "RVector.h"
#include "RRay.h"

#include <memory>

/// The result struct after a view ray bounces off a surface with materials
struct ViewRayBounceResult
{
	explicit ViewRayBounceResult(const RVec3& InAttenuation, const RVec3& InEmissive = RVec3(0, 0, 0))
		: Attenuation(InAttenuation)
		, Emissive(InEmissive)
	{
	}

	ViewRayBounceResult operator+(const ViewRayBounceResult& Rhs) const
	{
		return ViewRayBounceResult(Attenuation + Rhs.Attenuation, Emissive + Rhs.Emissive);
	}

	RVec3 Attenuation;
	RVec3 Emissive;
};

/// The surface material interface
class ISurfaceMaterial
{
public:
	virtual ~ISurfaceMaterial() {}

	/// Bounce a view ray against a surface of current material. Returns remaining amount of light after surface absorption.
	virtual ViewRayBounceResult BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const = 0;

	/// Get a preview color for this material used when rendering the base color
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const = 0;
};

/// Diffuse material
class SurfaceMaterial_Diffuse : public ISurfaceMaterial
{
public:
	SurfaceMaterial_Diffuse(const RVec3 InAlbedo = RVec3(1.0f, 1.0f, 1.0f));

	virtual ViewRayBounceResult BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const override;

protected:
	RVec3 Albedo;
};


class SurfaceMaterial_DiffuseChecker : public SurfaceMaterial_Diffuse
{
public:
	SurfaceMaterial_DiffuseChecker(const RVec3 InAlbedo = RVec3(1.0f, 1.0f, 1.0f), float InPatternSize = 5.0f);

	virtual ViewRayBounceResult BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const override;

private:
	bool IsBrighterArea(const RVec3& WorldPosition) const;
    
    // One divide pattern size
    float ReciprocalPatternSize;
};


/// Reflective material
class SurfaceMaterial_Reflective : public ISurfaceMaterial
{
public:
	SurfaceMaterial_Reflective(const RVec3 InAlbedo = RVec3(1.0f, 1.0f, 1.0f), float InFuzziness = 0.0f);

	virtual ViewRayBounceResult BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const override;
private:
	RVec3 Albedo;
	float Fuzziness;
};

/// Emissive surface material
class SurfaceMaterial_Emissive : public ISurfaceMaterial
{
public:
	SurfaceMaterial_Emissive(const RVec3 InColor);

	virtual ViewRayBounceResult BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const override;

private:
	RVec3 Color;
};


/// A blend material is used to blend two types of surface materials
class SurfaceMaterial_Blend : public ISurfaceMaterial
{
public:
	SurfaceMaterial_Blend(std::unique_ptr<ISurfaceMaterial> InMaterialA, std::unique_ptr<ISurfaceMaterial> InMaterialB, float InBlendFactor);

	virtual ViewRayBounceResult BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const override;
private:
	std::unique_ptr<ISurfaceMaterial> BlendMaterialA;
	std::unique_ptr<ISurfaceMaterial> BlendMaterialB;
	float BlendFactor;
};

class SurfaceMaterial_Combine : public ISurfaceMaterial
{
public:
	SurfaceMaterial_Combine(std::unique_ptr<ISurfaceMaterial> InMaterialA, std::unique_ptr<ISurfaceMaterial> InMaterialB);

	virtual ViewRayBounceResult BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const override;

private:
	std::unique_ptr<ISurfaceMaterial> MaterialA;
	std::unique_ptr<ISurfaceMaterial> MaterialB;
};

/// Surface material that allows light to pass through
class SurfaceMaterial_Null : public ISurfaceMaterial
{
public:
	SurfaceMaterial_Null()
	{
	}

	virtual ViewRayBounceResult BounceViewRay(const RRay& InViewRay, const RayHitResult& HitResult, RRay& OutViewRay) const override;
	virtual RVec3 PreviewColor(const RayHitResult& HitResult) const override;
};
